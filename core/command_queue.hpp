/**
 * @file command_queue.hpp
 * @brief 线程安全的指令下发队列
 *
 * 核心设计:
 *   - UI/Web 提交写指令到队列 (非阻塞)
 *   - 独立的命令处理线程从队列取指令执行
 *   - 写指令通过 SerialBus 与轮询线程共享总线 (互斥锁保证安全)
 *   - 写指令优先级高于读指令 (轮询会主动让步)
 *   - 每个指令有唯一ID, 支持查询执行结果
 *   - 支持超时和重试
 *
 * 这样采集(读)和下发(写)互不影响:
 *   - 轮询线程: 周期性读取设备数据
 *   - 命令线程: 处理UI下发的写指令
 *   - 两者通过 SerialBus 的互斥锁保证串口访问安全
 *   - 写指令获得总线后, 轮询自动等待 (不会丢失轮询, 只是延迟)
 */

#ifndef COMMAND_QUEUE_HPP
#define COMMAND_QUEUE_HPP

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include <map>
#include <cstdint>
#include <chrono>

/* 前向声明 */
class SerialBus;
class ModbusService;

/**
 * @brief 命令类型
 */
enum class CommandType {
    WRITE_REG,      // 写保持寄存器 (0x06)
    WRITE_COIL,     // 写线圈 (0x05)
    WRITE_MULTI_REG, // 写多个保持寄存器 (0x10)
    READ_REG,       // 读保持寄存器 (0x03) - 按需读取
    READ_COIL       // 读线圈 (0x01) - 按需读取
};

/**
 * @brief 命令优先级
 */
enum class CommandPriority {
    LOW     = 0,    // 低优先级 (普通轮询)
    NORMAL  = 1,    // 普通 (UI 操作)
    HIGH    = 2,    // 高优先级 (安全类设备)
    URGENT  = 3     // 紧急 (报警等)
};

/**
 * @brief 命令状态
 */
enum class CommandStatus {
    PENDING,        // 等待执行
    EXECUTING,      // 正在执行
    SUCCESS,        // 执行成功
    FAILED,         // 执行失败
    TIMEOUT,        // 执行超时
    CANCELLED       // 已取消
};

/**
 * @brief 命令结果
 */
struct CommandResult {
    CommandStatus status;
    int           errorCode;     // 0=成功, 其他=Modbus错误码
    std::string   errorMessage;
    uint8_t       responseData[256];
    size_t        responseLen;
    double        execTimeMs;    // 执行耗时 (毫秒)

    CommandResult()
        : status(CommandStatus::PENDING), errorCode(0),
          responseLen(0), execTimeMs(0.0) {}
};

/**
 * @brief 下发指令
 */
struct Command {
    uint64_t          id;            // 唯一ID
    CommandType       type;          // 命令类型
    CommandPriority   priority;      // 优先级
    uint8_t           devAddr;       // 设备地址 (1~247)
    uint16_t          regAddr;       // 寄存器/线圈地址
    uint16_t          value;         // 写入值
    uint16_t          count;         // 读取数量 (读操作用)
    int               timeoutMs;     // 超时 (毫秒)
    int               maxRetries;    // 最大重试次数
    std::string       description;   // 描述 (用于日志)

    /* 执行结果 (回调填充) */
    CommandResult     result;
    std::function<void(const CommandResult &)> callback;  // 完成回调

    Command()
        : id(0), type(CommandType::WRITE_REG),
          priority(CommandPriority::NORMAL),
          devAddr(0), regAddr(0), value(0), count(1),
          timeoutMs(2000), maxRetries(2) {}

    /* 优先级比较 (优先级高的排前面) */
    bool operator<(const Command &other) const {
        return static_cast<int>(priority) < static_cast<int>(other.priority);
    }
};

/**
 * @brief 命令队列统计
 */
struct CommandQueueStats {
    std::atomic<uint64_t> totalSubmitted{0};
    std::atomic<uint64_t> totalExecuted{0};
    std::atomic<uint64_t> totalSuccess{0};
    std::atomic<uint64_t> totalFailed{0};
    std::atomic<uint64_t> totalTimeout{0};
    std::atomic<double>   avgExecTimeMs{0.0};
    std::atomic<uint64_t> currentQueueSize{0};

    void reset() {
        totalSubmitted = 0;
        totalExecuted  = 0;
        totalSuccess   = 0;
        totalFailed    = 0;
        totalTimeout   = 0;
        avgExecTimeMs  = 0.0;
        currentQueueSize = 0;
    }
};

/**
 * @brief 线程安全的指令下发队列
 */
class CommandQueue {
public:
    /**
     * @brief 构造函数
     * @param bus  共享的串口总线
     * @param svc  Modbus 服务
     * @param workerCount 工作线程数 (默认 1, 通常1个就够)
     */
    CommandQueue(SerialBus *bus, ModbusService *svc, int workerCount = 1);
    ~CommandQueue();

    // 禁止拷贝
    CommandQueue(const CommandQueue &) = delete;
    CommandQueue &operator=(const CommandQueue &) = delete;

    /**
     * @brief 启动命令处理线程
     */
    void start();

    /**
     * @brief 停止命令处理线程
     */
    void stop();

    /**
     * @brief 提交写寄存器命令 (异步, 立即返回)
     * @param devAddr  设备地址
     * @param regAddr  寄存器地址
     * @param value    写入值
     * @param priority 优先级
     * @param callback 完成回调 (可选)
     * @return 命令ID
     */
    uint64_t writeRegister(uint8_t devAddr, uint16_t regAddr, uint16_t value,
                           CommandPriority priority = CommandPriority::NORMAL,
                           std::function<void(const CommandResult &)> callback = nullptr);

    /**
     * @brief 提交写线圈命令 (异步, 立即返回)
     * @param devAddr  设备地址
     * @param coilAddr 线圈地址
     * @param value    写入值 (true=ON, false=OFF)
     * @param priority 优先级
     * @param callback 完成回调 (可选)
     * @return 命令ID
     */
    uint64_t writeCoil(uint8_t devAddr, uint16_t coilAddr, bool value,
                       CommandPriority priority = CommandPriority::NORMAL,
                       std::function<void(const CommandResult &)> callback = nullptr);

    /**
     * @brief 提交通用命令
     * @param cmd 命令对象
     * @return 命令ID
     */
    uint64_t submit(const Command &cmd);

    /**
     * @brief 查询命令执行结果 (阻塞等待)
     * @param id  命令ID
     * @param timeoutMs 等待超时 (毫秒)
     * @return 命令结果
     */
    CommandResult waitResult(uint64_t id, int timeoutMs = 5000);

    /**
     * @brief 查询命令执行结果 (非阻塞)
     * @param id  命令ID
     * @param result 输出结果
     * @return true=有结果, false=还在执行中
     */
    bool queryResult(uint64_t id, CommandResult &result);

    /**
     * @brief 取消指定命令
     * @param id 命令ID
     * @return true=取消成功, false=命令不存在或已执行
     */
    bool cancel(uint64_t id);

    /**
     * @brief 清空队列中所有待执行的命令
     */
    void clear();

    /**
     * @brief 获取队列当前大小
     */
    size_t size() const;

    /**
     * @brief 获取统计信息
     */
    const CommandQueueStats &getStats() const { return stats_; }

    /**
     * @brief 是否正在运行
     */
    bool isRunning() const { return running_.load(); }

private:
    /**
     * @brief 命令处理线程函数
     */
    void workerThread(int threadId);

    /**
     * @brief 执行单个命令
     * @return true=成功, false=失败
     */
    bool executeCommand(Command &cmd);

    /**
     * @brief 生成唯一ID
     */
    uint64_t nextId();

    SerialBus                        *bus_;
    ModbusService                    *svc_;
    int                               workerCount_;

    /* 优先级队列 */
    std::priority_queue<Command>      queue_;
    mutable std::mutex                queueMtx_;
    std::condition_variable           queueCv_;

    /* 结果存储 */
    std::map<uint64_t, CommandResult> results_;
    mutable std::mutex                resultsMtx_;
    std::condition_variable           resultsCv_;

    /* 已取消的命令ID集合 */
    std::map<uint64_t, bool>          cancelled_;
    mutable std::mutex                cancelledMtx_;

    /* 线程控制 */
    std::atomic<bool>                 running_{false};
    std::vector<std::thread>          workers_;
    std::atomic<uint64_t>             idCounter_{0};
    CommandQueueStats                 stats_;
};

#endif /* COMMAND_QUEUE_HPP */