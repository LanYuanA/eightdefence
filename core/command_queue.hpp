/**
 * @file command_queue.hpp
 * @brief 异步指令下发队列
 *
 * 核心设计:
 *   - UI/Web 提交写指令到队列 (非阻塞)
 *   - 通过 AsyncBus 统一发送, 写操作优先于读操作
 *   - 每个指令有唯一ID, 支持查询执行结果
 *   - 支持超时和重试
 */

#ifndef COMMAND_QUEUE_HPP
#define COMMAND_QUEUE_HPP

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <map>
#include <cstdint>
#include <chrono>

class AsyncBus;
class ModbusService;

enum class CommandType {
    WRITE_REG, WRITE_COIL, READ_REG, READ_COIL
};

enum class CommandPriority {
    LOW = 0, NORMAL = 1, HIGH = 2, URGENT = 3
};

enum class CommandStatus {
    PENDING, EXECUTING, SUCCESS, FAILED, TIMEOUT, CANCELLED
};

struct CommandResult {
    CommandStatus status;
    int           errorCode;
    std::string   errorMessage;
    uint8_t       responseData[256];
    size_t        responseLen;
    double        execTimeMs;

    CommandResult()
        : status(CommandStatus::PENDING), errorCode(0),
          responseLen(0), execTimeMs(0.0) {}
};

struct Command {
    uint64_t          id;
    CommandType       type;
    CommandPriority   priority;
    uint8_t           devAddr;
    uint16_t          regAddr;
    uint16_t          value;
    uint16_t          count;
    int               timeoutMs;
    std::string       description;
    std::function<void(const CommandResult &)> callback;

    Command()
        : id(0), type(CommandType::WRITE_REG),
          priority(CommandPriority::NORMAL),
          devAddr(0), regAddr(0), value(0), count(1),
          timeoutMs(2000) {}
};

struct CommandQueueStats {
    std::atomic<uint64_t> totalSubmitted{0};
    std::atomic<uint64_t> totalSuccess{0};
    std::atomic<uint64_t> totalFailed{0};
    void reset() { totalSubmitted = 0; totalSuccess = 0; totalFailed = 0; }
};

/**
 * @brief 异步指令下发队列
 */
class CommandQueue {
public:
    CommandQueue(AsyncBus *asyncBus, ModbusService *svc);
    ~CommandQueue();

    CommandQueue(const CommandQueue &) = delete;
    CommandQueue &operator=(const CommandQueue &) = delete;

    void start();
    void stop();

    uint64_t writeRegister(uint8_t devAddr, uint16_t regAddr, uint16_t value,
                           CommandPriority priority = CommandPriority::NORMAL,
                           std::function<void(const CommandResult &)> callback = nullptr);

    uint64_t writeCoil(uint8_t devAddr, uint16_t coilAddr, bool value,
                       CommandPriority priority = CommandPriority::NORMAL,
                       std::function<void(const CommandResult &)> callback = nullptr);

    CommandResult waitResult(uint64_t id, int timeoutMs = 5000);
    bool queryResult(uint64_t id, CommandResult &result);
    bool cancel(uint64_t id);
    const CommandQueueStats &getStats() const { return stats_; }
    bool isRunning() const { return running_.load(); }

private:
    uint64_t submit(const Command &cmd);
    uint64_t nextId();
    void storeResult(uint64_t id, const CommandResult &result);

    AsyncBus                        *asyncBus_;
    ModbusService                    *svc_;
    std::atomic<bool>                running_{false};
    std::atomic<uint64_t>            idCounter_{0};

    std::map<uint64_t, CommandResult> results_;
    mutable std::mutex               resultsMtx_;
    std::condition_variable          resultsCv_;

    std::map<uint64_t, bool>         cancelled_;
    mutable std::mutex               cancelledMtx_;

    CommandQueueStats                stats_;
};

#endif /* COMMAND_QUEUE_HPP */
