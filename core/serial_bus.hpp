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

    void reset() {
        totalTransactions = 0;
        totalErrors = 0;
        totalBytesSent = 0;
        totalBytesRecv = 0;
        avgLatencyMs = 0.0;
        maxLatencyMs = 0.0;
        busContentionCount = 0;
    }
};

/**
 * @brief 线程安全的RS-485串口总线
 *
 * 所有对串口的访问都必须通过此类进行。
 * 内部使用 std::mutex 保证线程安全。
 * 串口 fd 持久化，避免频繁 open/close 的开销。
 */
class SerialBus {
public:
    /**
     * @brief 构造函数
     * @param device 串口设备路径 (如 "/dev/ttyS9")
     * @param baud   波特率 (默认 9600)
     */
    explicit SerialBus(const char *device, int baud = 9600);
    ~SerialBus();

    // 禁止拷贝
    SerialBus(const SerialBus &) = delete;
    SerialBus &operator=(const SerialBus &) = delete;

    /**
     * @brief 打开串口连接
     * @return 0=成功, 负值=失败
     */
    int open();

    /**
     * @brief 关闭串口连接
     */
    void close();

    /**
     * @brief 检查串口是否已打开
     */
    bool isOpen() const { return fd_ >= 0; }

    /**
     * @brief 线程安全的 Modbus 事务 (发送请求 + 接收响应)
     *
     * 整个事务在 mutex 保护下原子执行:
     *   1. 获取互斥锁
     *   2. 清空接收缓冲区 (丢弃残留数据)
     *   3. 发送请求帧
     *   4. 等待响应 (带超时)
     *   5. 释放互斥锁
     *
     * @param request      请求数据 (含CRC)
     * @param request_len  请求数据长度
     * @param response     响应缓冲区
     * @param response_cap 响应缓冲区容量
     * @param response_len 实际接收的响应长度
     * @param timeout_ms   接收超时 (毫秒)
     * @return 0=成功, 负值=失败
     */
    int transact(const uint8_t *request, size_t request_len,
                 uint8_t *response, size_t response_cap,
                 size_t *response_len, int timeout_ms);

    /**
     * @brief 带HEX字符串的事务接口 (兼容现有代码)
     */
    int transactHex(const char *hex_cmd,
                    uint8_t *response, size_t response_cap,
                    size_t *response_len, int timeout_ms);

    /**
     * @brief 获取通信统计
     */
    const BusStats &getStats() const { return stats_; }

    /**
     * @brief 重置统计信息
     */
    void resetStats() { stats_.reset(); }

    /**
     * @brief 获取串口设备路径
     */
    const std::string &getDevice() const { return device_; }

    /**
     * @brief 获取波特率
     */
    int getBaud() const { return baud_; }

    /**
     * @brief 设置帧间最小间隔 (微秒)
     *
     * Modbus RTU 要求 3.5 字符时间的帧间间隔。
     * 在 9600 波特率下约为 4ms。默认设置为 5ms 以留有余量。
     */
    void setInterFrameDelayUs(uint32_t us) { inter_frame_delay_us_ = us; }

private:
    /**
     * @brief 内部: 发送数据
     */
    int rawSend(const uint8_t *data, size_t len);

    /**
     * @brief 内部: 接收数据 (带超时)
     */
    int rawRecv(uint8_t *buf, size_t buf_cap, size_t *recv_len, int timeout_ms);

    /**
     * @brief 内部: 清空串口缓冲区
     */
    void flushBuffers();

    /**
     * @brief 内部: 更新延迟统计
     */
    void updateLatencyStats(double latency_ms);

    std::string device_;
    int         baud_;
    int         fd_ = -1;
    std::recursive_mutex mtx_;  // 使用 recursive_mutex 防止重入死锁
    uint32_t    inter_frame_delay_us_ = 5000;  // 帧间延时 5ms
    BusStats    stats_;
    bool        auto_reconnect_ = true;
};

#endif /* SERIAL_BUS_HPP */