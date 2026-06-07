/**
 * @file serial_bus.hpp
 * @brief 线程安全的RS-485串口总线管理器
 *
 * 核心设计:
 *   - 持久化串口文件描述符 (避免每次通信都 open/close)
 *   - 互斥锁保护 (保证同一时刻只有一个线程占用总线)
 *   - 帧间延时 (满足 Modbus RTU 3.5字符时间要求)
 *   - 通信统计 (事务数、错误数、平均延迟)
 *   - 自动重连 (串口异常时自动恢复)
 *   - 写优先: 写操作会设置标志, 读操作检测到写等待时主动让步
 */

#ifndef SERIAL_BUS_HPP
#define SERIAL_BUS_HPP

#include <cstdint>
#include <cstddef>
#include <mutex>
#include <string>
#include <atomic>
#include <chrono>

/**
 * @brief 通信统计信息
 */
struct BusStats {
    std::atomic<uint64_t> totalTransactions{0};
    std::atomic<uint64_t> totalErrors{0};
    std::atomic<uint64_t> totalBytesSent{0};
    std::atomic<uint64_t> totalBytesRecv{0};
    std::atomic<double>   avgLatencyMs{0.0};
    std::atomic<double>   maxLatencyMs{0.0};
    std::atomic<uint64_t> busContentionCount{0};  // 总线争用次数
    std::atomic<uint64_t> writePreemptCount{0};   // 写指令抢占次数

    void reset() {
        totalTransactions = 0;
        totalErrors = 0;
        totalBytesSent = 0;
        totalBytesRecv = 0;
        avgLatencyMs = 0.0;
        maxLatencyMs = 0.0;
        busContentionCount = 0;
        writePreemptCount = 0;
    }
};

/**
 * @brief 线程安全的RS-485串口总线
 *
 * 所有对串口的访问都必须通过此类进行。
 * 内部使用 std::mutex 保证线程安全。
 * 串口 fd 持久化，避免频繁 open/close 的开销。
 * 写操作优先: 写操作设置 writePending 标志, 读操作检测到时主动让步。
 */
class SerialBus {
public:
    explicit SerialBus(const char *device, int baud = 9600);
    ~SerialBus();

    SerialBus(const SerialBus &) = delete;
    SerialBus &operator=(const SerialBus &) = delete;

    int open();
    void close();
    bool isOpen() const { return fd_ >= 0; }

    /**
     * @brief 线程安全的 Modbus 事务 (发送请求 + 接收响应)
     *
     * 整个事务在 mutex 保护下原子执行:
     *   1. 获取独占锁 (读写互斥, 保证半双工总线安全)
     *   2. 写优先: 读操作检测到 writePending 时等待最多 50ms 让写先执行
     *   3. 清空接收缓冲区 (丢弃残留数据)
     *   4. 发送请求帧
     *   5. 等待响应 (带超时)
     *   6. 释放锁
     *
     * @param request      请求数据 (含CRC)
     * @param request_len  请求数据长度
     * @param response     响应缓冲区
     * @param response_cap 响应缓冲区容量
     * @param response_len 实际接收的响应长度
     * @param timeout_ms   接收超时 (毫秒)
     * @param isWrite      是否为写操作 (写操作获得优先权)
     * @return 0=成功, 负值=失败
     */
    int transact(const uint8_t *request, size_t request_len,
                 uint8_t *response, size_t response_cap,
                 size_t *response_len, int timeout_ms,
                 bool isWrite = false);

    int transactHex(const char *hex_cmd,
                    uint8_t *response, size_t response_cap,
                    size_t *response_len, int timeout_ms,
                    bool isWrite = false);

    const BusStats &getStats() const { return stats_; }
    void resetStats() { stats_.reset(); }
    const std::string &getDevice() const { return device_; }
    int getBaud() const { return baud_; }
    void setInterFrameDelayUs(uint32_t us) { inter_frame_delay_us_ = us; }

private:
    int rawSend(const uint8_t *data, size_t len);
    int rawRecv(uint8_t *buf, size_t buf_cap, size_t *recv_len, int timeout_ms);
    void flushBuffers();
    void updateLatencyStats(double latency_ms);

    std::string device_;
    int         baud_;
    int         fd_ = -1;
    std::mutex  mtx_;                               // 独占锁, 保证半双工安全
    std::atomic<int>        writePendingCount_{0};   // 待执行的写操作计数
    uint32_t    inter_frame_delay_us_ = 5000;        // 帧间延时 5ms
    BusStats    stats_;
    bool        auto_reconnect_ = true;
    int         shutdown_pipe_[2] = {-1, -1};        // 用于中断 select() 的管道
};

#endif /* SERIAL_BUS_HPP */
