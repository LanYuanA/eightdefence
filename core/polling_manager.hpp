/**
 * @file polling_manager.hpp
 * @brief 多线程设备轮询管理器
 *
 * 核心设计:
 *   - 每个设备任务独立线程, 可自定义轮询间隔
 *   - 所有线程共享同一个 SerialBus 实例 (线程安全)
 *   - 高优先级任务 (烟雾/水浸/红外等安全设备) 可配置更快的轮询间隔
 *   - 支持任务分组: 同一物理设备的多个传感器任务按顺序轮询 (避免同一设备并发)
 *   - 线程生命周期管理: 优雅启动/停止
 *   - 内置性能监控: 实时查看各任务轮询频率和延迟
 */

#ifndef POLLING_MANAGER_HPP
#define POLLING_MANAGER_HPP

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <chrono>
#include <cstdint>

#include "devices/device_base.hpp"

class SerialBus;
class ModbusService;
struct BusStats;

/**
 * @brief 单个轮询任务的运行时统计
 */
struct TaskStats {
    std::atomic<uint64_t> pollCount{0};
    std::atomic<uint64_t> successCount{0};
    std::atomic<uint64_t> failCount{0};
    std::atomic<double>   avgPollTimeMs{0.0};
    std::atomic<double>   maxPollTimeMs{0.0};
    std::atomic<double>   lastPollTimeMs{0.0};

    TaskStats() = default;

    // 拷贝构造/赋值: 手动加载/存储 atomic 值 (允许 vector 操作)
    TaskStats(const TaskStats &other)
        : pollCount(other.pollCount.load()),
          successCount(other.successCount.load()),
          failCount(other.failCount.load()),
          avgPollTimeMs(other.avgPollTimeMs.load()),
          maxPollTimeMs(other.maxPollTimeMs.load()),
          lastPollTimeMs(other.lastPollTimeMs.load()) {}

    TaskStats &operator=(const TaskStats &other) {
        if (this != &other) {
            pollCount.store(other.pollCount.load());
            successCount.store(other.successCount.load());
            failCount.store(other.failCount.load());
            avgPollTimeMs.store(other.avgPollTimeMs.load());
            maxPollTimeMs.store(other.maxPollTimeMs.load());
            lastPollTimeMs.store(other.lastPollTimeMs.load());
        }
        return *this;
    }

    void reset() {
        pollCount = 0;
        successCount = 0;
        failCount = 0;
        avgPollTimeMs = 0.0;
        maxPollTimeMs = 0.0;
        lastPollTimeMs = 0.0;
    }
};

/**
 * @brief 任务组配置
 *
 * 将同一物理设备的多个传感器任务分组, 组内串行轮询, 组间并行。
 * 例如: 云测仪的 PM2.5/PM10/湿度/温度/... 共享地址 0x30, 应在同一组内。
 */
struct TaskGroup {
    std::string              name;           // 组名 (如 "云测仪", "恒湿机")
    std::vector<DeviceTask>  tasks;          // 组内的任务列表
    int                      pollIntervalMs; // 整组的轮询间隔 (取组内任务最大值)
    int                      priority;       // 组优先级
    uint8_t                  devAddr;        // 物理设备地址

    TaskGroup() : pollIntervalMs(5000), priority(0), devAddr(0) {}
};

/**
 * @brief 轮询管理器配置
 */
struct PollingConfig {
    int maxConcurrentGroups = 8;     // 最大并发组数 (线程数)
    int busLockTimeoutMs    = 5000;  // 总线锁超时
    bool enablePriorityBoost = true; // 启用优先级提升 (安全类设备优先)
    int  priorityBoostMs    = 100;   // 高优先级任务的额外等待精度
};

/**
 * @brief 多线程设备轮询管理器
 *
 * 管理所有设备轮询任务的生命周期:
 *   1. 将任务自动分组 (按物理设备地址)
 *   2. 每组分配一个独立线程
 *   3. 线程安全地访问串口总线
 *   4. 支持运行时调整轮询间隔
 *   5. 支持性能监控和压力测试
 */
class PollingManager {
public:
    /**
     * @brief 构造函数
     * @param bus   共享的串口总线实例
     * @param svc   Modbus服务实例
     * @param config 轮询配置
     */
    PollingManager(SerialBus *bus, ModbusService *svc,
                   const PollingConfig &config = PollingConfig());
    ~PollingManager();

    // 禁止拷贝
    PollingManager(const PollingManager &) = delete;
    PollingManager &operator=(const PollingManager &) = delete;

    /**
     * @brief 添加设备任务 (自动按设备地址分组)
     * @param tasks 设备任务列表
     */
    void addTasks(const std::vector<DeviceTask> &tasks);

    /**
     * @brief 手动添加任务组
     * @param group 任务组
     */
    void addGroup(const TaskGroup &group);

    /**
     * @brief 启动所有轮询线程
     * @return 0=成功
     */
    int start();

    /**
     * @brief 停止所有轮询线程 (优雅退出)
     * @param timeoutSec 等待超时 (秒), 0=不等待
     */
    void stop(int timeoutSec = 5);

    /**
     * @brief 检查是否正在运行
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief 统计快照 (可拷贝, 用于返回)
     */
    struct TaskStatsSnapshot {
        uint64_t pollCount;
        uint64_t successCount;
        uint64_t failCount;
        double   avgPollTimeMs;
        double   maxPollTimeMs;
        double   lastPollTimeMs;
    };

    /**
     * @brief 获取所有任务组的统计快照
     */
    std::map<std::string, TaskStatsSnapshot> getGroupStatsSnapshot() const;

    /**
     * @brief 获取总线统计信息
     */
    const BusStats &getBusStats() const;

    /**
     * @brief 打印实时状态报告
     */
    void printStatus() const;

    /**
     * @brief 修改指定组的轮询间隔
     * @param groupName 组名
     * @param intervalMs 新的轮询间隔 (毫秒)
     * @return 0=成功, -1=未找到
     */
    int setGroupInterval(const std::string &groupName, int intervalMs);

    /**
     * @brief 获取当前任务组数量
     */
    size_t getGroupCount() const { return groups_.size(); }

    /**
     * @brief 获取当前活跃线程数
     */
    int getActiveThreadCount() const;

private:
    /**
     * @brief 单个任务组的轮询线程函数
     */
    void pollGroupThread(size_t groupIndex);

    /**
     * @brief 执行单次任务组轮询
     */
    void pollGroupOnce(size_t groupIndex);

    SerialBus                *bus_;
    ModbusService            *svc_;
    PollingConfig            config_;
    std::atomic<bool>        running_{false};
    std::vector<TaskGroup>   groups_;
    std::vector<std::thread> threads_;
    std::vector<TaskStats>   groupStats_;
    std::atomic<int>         activeThreads_{0};

    // 组间互斥: 同一物理设备的任务在同一时刻只能有一个在总线上
    // 不同物理设备的任务可以并行 (通过 SerialBus 的内部 mutex 协调)
};

#endif /* POLLING_MANAGER_HPP */