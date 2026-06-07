/**
 * @file async_bus.hpp
 * @brief 异步串口总线 - 请求/响应分离架构
 *
 * 核心设计:
 *   - 所有设备共享一个 I/O 线程, 串行发送请求并接收响应
 *   - 调用方提交请求+回调, 不阻塞等待
 *   - 响应到达后自动调用对应的回调函数
 *   - 写操作优先于读操作 (写优先队列)
 *   - 统一管理所有设备的总线访问, 消除多线程争用
 *
 * 使用方式:
 *   AsyncBus asyncBus(&serialBus);
 *   asyncBus.start();
 *
 *   // 提交读请求 (非阻塞)
 *   AsyncRequest req;
 *   req.buildReadReg(0x30, 0x0000, 1);  // 云测仪 PM2.5
 *   req.callback = [](const uint8_t *resp, size_t len, int rc) {
 *       if (rc == 0) { // 解析响应 }
 *   };
 *   asyncBus.submit(req);
 */

#ifndef ASYNC_BUS_HPP
#define ASYNC_BUS_HPP

#include <cstdint>
#include <cstddef>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <string>

class SerialBus;

/**
 * @brief 异步请求
 */
struct AsyncRequest {
    uint8_t     data[256];      // Modbus RTU 帧数据
    size_t      len = 0;        // 帧长度
    bool        isWrite = false;// 是否为写操作 (写优先)
    int         timeoutMs = 1000;   // 响应超时
    int         maxRetries = 0;     // 最大重试次数
    std::string description;        // 描述 (用于日志)

    // 回调: resp=响应数据, resp_len=响应长度, rc=0成功/负值失败
    std::function<void(const uint8_t *resp, size_t resp_len, int rc)> callback;

    // 便捷方法: 构建读寄存器帧
    void buildReadReg(uint8_t devAddr, uint16_t regAddr, uint16_t count);
    // 便捷方法: 构建写寄存器帧
    void buildWriteReg(uint8_t devAddr, uint16_t regAddr, uint16_t value);
    // 便捷方法: 构建读线圈帧
    void buildReadCoil(uint8_t devAddr, uint16_t coilAddr, uint16_t count);
    // 便捷方法: 构建写线圈帧
    void buildWriteCoil(uint8_t devAddr, uint16_t coilAddr, bool value);

    // 优先级比较 (写操作优先)
    bool operator<(const AsyncRequest &other) const {
        return isWrite < other.isWrite;  // true=写, 写排前面
    }
};

/**
 * @brief 异步总线统计
 */
struct AsyncBusStats {
    std::atomic<uint64_t> totalSubmitted{0};
    std::atomic<uint64_t> totalProcessed{0};
    std::atomic<uint64_t> totalSuccess{0};
    std::atomic<uint64_t> totalFailed{0};
    std::atomic<uint64_t> totalRetries{0};
    std::atomic<uint64_t> totalWriteOps{0};
    std::atomic<double>   avgTransactMs{0.0};
    std::atomic<uint64_t> queueHighWaterMark{0};

    void reset() {
        totalSubmitted = 0;
        totalProcessed = 0;
        totalSuccess = 0;
        totalFailed = 0;
        totalRetries = 0;
        totalWriteOps = 0;
        avgTransactMs = 0.0;
        queueHighWaterMark = 0;
    }
};

/**
 * @brief 异步串口总线
 */
class AsyncBus {
public:
    explicit AsyncBus(SerialBus *bus);
    ~AsyncBus();

    AsyncBus(const AsyncBus &) = delete;
    AsyncBus &operator=(const AsyncBus &) = delete;

    /**
     * @brief 启动 I/O 线程
     */
    void start();

    /**
     * @brief 停止 I/O 线程
     */
    void stop();

    /**
     * @brief 提交异步请求 (非阻塞, 线程安全)
     * @param req 请求对象
     */
    void submit(const AsyncRequest &req);

    /**
     * @brief 是否正在运行
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief 获取统计信息
     */
    const AsyncBusStats &getStats() const { return stats_; }

    /**
     * @brief 获取当前队列大小
     */
    size_t getQueueSize() const;

private:
    /**
     * @brief I/O 线程主循环
     */
    void ioThread();

    SerialBus                       *bus_;
    std::priority_queue<AsyncRequest> queue_;
    mutable std::mutex              queueMtx_;
    std::condition_variable         queueCv_;

    std::atomic<bool>               running_{false};
    std::thread                     ioThread_;
    AsyncBusStats                   stats_;
};

#endif /* ASYNC_BUS_HPP */
