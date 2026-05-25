/**
 * @file polling_manager.cpp
 * @brief 多线程设备轮询管理器实现
 */

#include "polling_manager.hpp"
#include "serial_bus.hpp"
#include "service/modbus_service.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <thread>
#include <chrono>

/* ============================================================
 * 构造 / 析构
 * ============================================================ */
PollingManager::PollingManager(SerialBus *bus, ModbusService *svc,
                               const PollingConfig &config)
    : bus_(bus), svc_(svc), config_(config) {}

PollingManager::~PollingManager() {
    stop(3);
}

/* ============================================================
 * 任务管理
 * ============================================================ */
void PollingManager::addTasks(const std::vector<DeviceTask> &tasks) {
    // 按 devAddr 自动分组
    // devAddr=0 的任务视为独立任务, 每个任务一组
    // devAddr 相同的任务归入同一组 (组内串行轮询)
    std::map<uint8_t, TaskGroup> addrGroups;
    std::vector<TaskGroup> standaloneGroups;

    for (const auto &task : tasks) {
        if (task.devAddr == 0) {
            // 独立任务, 每个一组
            TaskGroup g;
            g.name = task.description;
            g.tasks.push_back(task);
            g.pollIntervalMs = task.pollIntervalMs;
            g.priority = task.priority;
            g.devAddr = task.devAddr;
            standaloneGroups.push_back(g);
        } else {
            auto it = addrGroups.find(task.devAddr);
            if (it == addrGroups.end()) {
                TaskGroup g;
                g.name = task.description;  // 组名用第一个任务的描述
                g.devAddr = task.devAddr;
                g.priority = task.priority;
                addrGroups[task.devAddr] = g;
            }
            addrGroups[task.devAddr].tasks.push_back(task);
            // 更新组的轮询间隔: 取组内所有任务的最小间隔
            addrGroups[task.devAddr].pollIntervalMs =
                std::min(addrGroups[task.devAddr].pollIntervalMs, task.pollIntervalMs);
            // 更新优先级: 取最高优先级
            addrGroups[task.devAddr].priority =
                std::max(addrGroups[task.devAddr].priority, task.priority);
        }
    }

    for (auto &pair : addrGroups) {
        groups_.push_back(pair.second);
    }
    for (auto &g : standaloneGroups) {
        groups_.push_back(g);
    }
}

void PollingManager::addGroup(const TaskGroup &group) {
    groups_.push_back(group);
}

/* ============================================================
 * 启动 / 停止
 * ============================================================ */
int PollingManager::start() {
    if (running_.load()) {
        fprintf(stderr, "[PollingManager] 已在运行中\n");
        return -1;
    }

    if (groups_.empty()) {
        fprintf(stderr, "[PollingManager] 没有任务组\n");
        return -2;
    }

    if (!bus_ || !bus_->isOpen()) {
        fprintf(stderr, "[PollingManager] 串口总线未打开\n");
        return -3;
    }

    // 初始化统计
    groupStats_.clear();
    groupStats_.resize(groups_.size());

    // 按优先级排序: 高优先级的组排在前面 (先启动)
    std::vector<size_t> order(groups_.size());
    for (size_t i = 0; i < order.size(); i++) order[i] = i;
    std::sort(order.begin(), order.end(), [this](size_t a, size_t b) {
        return groups_[a].priority > groups_[b].priority;
    });

    running_.store(true);
    activeThreads_.store(0);

    printf("========================================\n");
    printf("  PollingManager 启动\n");
    printf("  任务组数: %zu\n", groups_.size());
    printf("========================================\n");

    for (size_t idx = 0; idx < order.size(); idx++) {
        size_t gi = order[idx];
        const auto &g = groups_[gi];
        printf("  [%zu] 组 '%s' - 地址=0x%02X, 间隔=%dms, 优先级=%d, 任务数=%zu\n",
               idx, g.name.c_str(), g.devAddr, g.pollIntervalMs,
               g.priority, g.tasks.size());

        threads_.emplace_back(&PollingManager::pollGroupThread, this, gi);
    }

    printf("========================================\n");

    return 0;
}

void PollingManager::stop(int timeoutSec) {
    if (!running_.load()) return;

    printf("[PollingManager] 正在停止所有轮询线程...\n");
    running_.store(false);

    if (timeoutSec > 0) {
        auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(timeoutSec);

        for (auto &t : threads_) {
            if (t.joinable()) {
                auto remaining = deadline - std::chrono::steady_clock::now();
                if (remaining > std::chrono::seconds(0)) {
                    t.join();
                } else {
                    t.detach();  // 超时, 分离线程
                    fprintf(stderr, "[PollingManager] 警告: 线程未在超时内退出\n");
                }
            }
        }
    } else {
        for (auto &t : threads_) {
            if (t.joinable()) t.detach();
        }
    }

    threads_.clear();
    activeThreads_.store(0);
    printf("[PollingManager] 所有轮询线程已停止\n");
}

/* ============================================================
 * 轮询线程函数
 * ============================================================ */
void PollingManager::pollGroupThread(size_t groupIndex) {
    activeThreads_++;
    const auto &group = groups_[groupIndex];

    printf("[PollingManager] 线程启动: 组 '%s' (地址=0x%02X), 间隔=%dms\n",
           group.name.c_str(), group.devAddr, group.pollIntervalMs);

    while (running_.load()) {
        auto loopStart = std::chrono::steady_clock::now();

        // 执行一次轮询
        pollGroupOnce(groupIndex);

        // 等待到下一次轮询时间
        auto loopEnd = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            loopEnd - loopStart).count();

        int sleepMs = group.pollIntervalMs - static_cast<int>(elapsed);
        if (sleepMs > 0 && running_.load()) {
            // 分段睡眠, 以便能及时响应停止信号
            int remaining = sleepMs;
            while (remaining > 0 && running_.load()) {
                int chunk = std::min(remaining, 100);  // 每100ms检查一次
                std::this_thread::sleep_for(std::chrono::milliseconds(chunk));
                remaining -= chunk;
            }
        }
    }

    activeThreads_--;
    printf("[PollingManager] 线程退出: 组 '%s'\n", group.name.c_str());
}

void PollingManager::pollGroupOnce(size_t groupIndex) {
    const auto &group = groups_[groupIndex];
    auto &stats = groupStats_[groupIndex];

    for (const auto &task : group.tasks) {
        if (!running_.load()) break;

        auto taskStart = std::chrono::steady_clock::now();

        stats.pollCount++;

        // 读取数据
        uint8_t resp[256] = {0};
        size_t resp_len = 0;
        int rc = task.readFunc(*svc_, resp, &resp_len);

        // 处理数据
        if (task.processFunc) {
            task.processFunc(resp, resp_len, rc);
        }

        if (rc == 0) {
            stats.successCount++;
        } else {
            stats.failCount++;
        }

        // 更新统计
        auto taskEnd = std::chrono::steady_clock::now();
        double taskMs = std::chrono::duration<double, std::milli>(
            taskEnd - taskStart).count();

        stats.lastPollTimeMs.store(taskMs);

        double oldAvg = stats.avgPollTimeMs.load();
        stats.avgPollTimeMs.store(oldAvg * 0.9 + taskMs * 0.1);

        double oldMax = stats.maxPollTimeMs.load();
        while (taskMs > oldMax) {
            if (stats.maxPollTimeMs.compare_exchange_weak(oldMax, taskMs)) break;
        }
    }
}

/* ============================================================
 * 统计和监控
 * ============================================================ */
std::map<std::string, PollingManager::TaskStatsSnapshot> PollingManager::getGroupStatsSnapshot() const {
    std::map<std::string, TaskStatsSnapshot> result;
    for (size_t i = 0; i < groups_.size() && i < groupStats_.size(); i++) {
        TaskStatsSnapshot snap;
        snap.pollCount      = groupStats_[i].pollCount.load();
        snap.successCount   = groupStats_[i].successCount.load();
        snap.failCount      = groupStats_[i].failCount.load();
        snap.avgPollTimeMs  = groupStats_[i].avgPollTimeMs.load();
        snap.maxPollTimeMs  = groupStats_[i].maxPollTimeMs.load();
        snap.lastPollTimeMs = groupStats_[i].lastPollTimeMs.load();
        result[groups_[i].name] = snap;
    }
    return result;
}

const BusStats &PollingManager::getBusStats() const {
    return bus_->getStats();
}

void PollingManager::printStatus() const {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              轮询管理器状态报告                              ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ 运行状态: %-10s  活跃线程: %-3d  任务组: %-3zu          ║\n",
           running_.load() ? "运行中" : "已停止",
           activeThreads_.load(), groups_.size());

    const auto &bs = bus_->getStats();
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ 总线统计                                                     ║\n");
    printf("║   总事务数: %-10lu  错误数: %-10lu                   ║\n",
           (unsigned long)bs.totalTransactions.load(),
           (unsigned long)bs.totalErrors.load());
    printf("║   发送字节: %-10lu  接收字节: %-10lu                 ║\n",
           (unsigned long)bs.totalBytesSent.load(),
           (unsigned long)bs.totalBytesRecv.load());
    printf("║   平均延迟: %-8.1fms  最大延迟: %-8.1fms              ║\n",
           bs.avgLatencyMs.load(), bs.maxLatencyMs.load());
    printf("║   总线争用次数: %-8lu                                  ║\n",
           (unsigned long)bs.busContentionCount.load());

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ 任务组详情                                                   ║\n");

    for (size_t i = 0; i < groups_.size() && i < groupStats_.size(); i++) {
        const auto &g = groups_[i];
        const auto &s = groupStats_[i];
        double successRate = s.pollCount > 0 ?
            (double)s.successCount.load() / s.pollCount.load() * 100.0 : 0.0;

        printf("║   [%zu] %-16s addr=0x%02X interval=%dms               ║\n",
               i, g.name.c_str(), g.devAddr, g.pollIntervalMs);
        printf("║       轮询:%-8lu 成功:%-8lu 失败:%-8lu 成功率:%.1f%%  ║\n",
               (unsigned long)s.pollCount.load(),
               (unsigned long)s.successCount.load(),
               (unsigned long)s.failCount.load(),
               successRate);
        printf("║       平均耗时:%.1fms  最大耗时:%.1fms  上次耗时:%.1fms  ║\n",
               s.avgPollTimeMs.load(), s.maxPollTimeMs.load(),
               s.lastPollTimeMs.load());
    }

    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int PollingManager::setGroupInterval(const std::string &groupName, int intervalMs) {
    for (auto &g : groups_) {
        if (g.name == groupName) {
            printf("[PollingManager] 修改组 '%s' 轮询间隔: %dms -> %dms\n",
                   g.name.c_str(), g.pollIntervalMs, intervalMs);
            g.pollIntervalMs = intervalMs;
            return 0;
        }
    }
    return -1;
}

int PollingManager::getActiveThreadCount() const {
    return activeThreads_.load();
}