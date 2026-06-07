/**
 * @file polling_manager.hpp
 * @brief 异步设备轮询管理器
 *
 * 核心设计:
 *   - 单调度线程, 按间隔向 AsyncBus 提交请求 (非阻塞)
 *   - 响应到达后由 AsyncBus I/O 线程调用 processFunc 回调
 *   - 支持任务分组: 同一物理设备的任务按顺序提交
 *   - 支持运行时调整轮询间隔
 *   - 内置性能监控
 */

#ifndef POLLING_MANAGER_HPP
#define POLLING_MANAGER_HPP

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include <chrono>
#include <cstdint>

#include "devices/device_base.hpp"

class AsyncBus;

/**
 * @brief 单个任务组的运行时统计
 */
struct TaskStats {
    std::atomic<uint64_t> pollCount{0};
    std::atomic<uint64_t> successCount{0};
    std::atomic<uint64_t> failCount{0};

    TaskStats() = default;
    TaskStats(const TaskStats &other)
        : pollCount(other.pollCount.load()),
          successCount(other.successCount.load()),
          failCount(other.failCount.load()) {}
    TaskStats &operator=(const TaskStats &other) {
        if (this != &other) {
            pollCount.store(other.pollCount.load());
            successCount.store(other.successCount.load());
            failCount.store(other.failCount.load());
        }
        return *this;
    }
};

/**
 * @brief 任务组
 */
struct TaskGroup {
    std::string              name;
    std::vector<DeviceTask>  tasks;
    int                      pollIntervalMs;
    int                      priority;
    uint8_t                  devAddr;
    TaskGroup() : pollIntervalMs(5000), priority(0), devAddr(0) {}
};

/**
 * @brief 异步轮询管理器
 */
class PollingManager {
public:
    PollingManager(AsyncBus *asyncBus);
    ~PollingManager();

    PollingManager(const PollingManager &) = delete;
    PollingManager &operator=(const PollingManager &) = delete;

    void addTasks(const std::vector<DeviceTask> &tasks);
    void addGroup(const TaskGroup &group);
    int start();
    void stop(int timeoutSec = 5);
    bool isRunning() const { return running_.load(); }

    struct TaskStatsSnapshot {
        uint64_t pollCount;
        uint64_t successCount;
        uint64_t failCount;
    };
    std::map<std::string, TaskStatsSnapshot> getGroupStatsSnapshot() const;
    void printStatus() const;
    int setGroupInterval(const std::string &groupName, int intervalMs);
    size_t getGroupCount() const { return groups_.size(); }

private:
    void schedulerThread();

    AsyncBus                *asyncBus_;
    std::atomic<bool>       running_{false};
    std::vector<TaskGroup>  groups_;
    std::vector<TaskStats>  groupStats_;
    std::thread             schedulerThread_;
};

#endif /* POLLING_MANAGER_HPP */
