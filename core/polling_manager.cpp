/**
 * @file polling_manager.cpp
 * @brief 异步设备轮询管理器实现
 */

#include "polling_manager.hpp"
#include "async_bus.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>

/* ============================================================
 * 构造 / 析构
 * ============================================================ */
PollingManager::PollingManager(AsyncBus *asyncBus)
    : asyncBus_(asyncBus) {}

PollingManager::~PollingManager() {
    stop(3);
}

/* ============================================================
 * 任务管理
 * ============================================================ */
void PollingManager::addTasks(const std::vector<DeviceTask> &tasks) {
    std::map<uint8_t, TaskGroup> addrGroups;
    std::vector<TaskGroup> standaloneGroups;

    for (const auto &task : tasks) {
        if (task.devAddr == 0) {
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
                g.name = task.description;
                g.devAddr = task.devAddr;
                g.priority = task.priority;
                addrGroups[task.devAddr] = g;
            }
            addrGroups[task.devAddr].tasks.push_back(task);
            addrGroups[task.devAddr].pollIntervalMs =
                std::min(addrGroups[task.devAddr].pollIntervalMs, task.pollIntervalMs);
            addrGroups[task.devAddr].priority =
                std::max(addrGroups[task.devAddr].priority, task.priority);
        }
    }

    for (auto &pair : addrGroups) groups_.push_back(pair.second);
    for (auto &g : standaloneGroups) groups_.push_back(g);
}

void PollingManager::addGroup(const TaskGroup &group) {
    groups_.push_back(group);
}

/* ============================================================
 * 启动 / 停止
 * ============================================================ */
int PollingManager::start() {
    if (running_.load()) return -1;
    if (groups_.empty()) return -2;

    groupStats_.clear();
    groupStats_.resize(groups_.size());

    running_ = true;
    schedulerThread_ = std::thread(&PollingManager::schedulerThread, this);

    printf("========================================\n");
    printf("  异步 PollingManager 启动\n");
    printf("  任务组数: %zu\n", groups_.size());
    for (size_t i = 0; i < groups_.size(); i++) {
        const auto &g = groups_[i];
        printf("  [%zu] 组 '%s' - 地址=0x%02X, 间隔=%dms, 任务数=%zu\n",
               i, g.name.c_str(), g.devAddr, g.pollIntervalMs, g.tasks.size());
    }
    printf("========================================\n");
    return 0;
}

void PollingManager::stop(int timeoutSec) {
    if (!running_.load()) return;
    running_ = false;
    if (schedulerThread_.joinable()) {
        if (timeoutSec > 0) {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);
            // 唤醒调度线程 (它可能在 sleep)
            schedulerThread_.join();
        } else {
            schedulerThread_.detach();
        }
    }
}

/* ============================================================
 * 调度线程
 * ============================================================ */
void PollingManager::schedulerThread() {
    printf("[PollingManager] 调度线程启动\n");

    // 每组的下次轮询时间
    std::vector<std::chrono::steady_clock::time_point> nextPoll(groups_.size());
    auto now = std::chrono::steady_clock::now();
    for (size_t i = 0; i < groups_.size(); i++) {
        // 错开启动时间, 避免所有组同时提交
        nextPoll[i] = now + std::chrono::milliseconds(i * 50);
    }

    while (running_.load()) {
        now = std::chrono::steady_clock::now();

        for (size_t gi = 0; gi < groups_.size(); gi++) {
            if (!running_.load()) break;
            if (now < nextPoll[gi]) continue;

            const auto &group = groups_[gi];
            auto &stats = groupStats_[gi];

            // 提交组内所有任务到 AsyncBus
            for (const auto &task : group.tasks) {
                if (!running_.load()) break;

                // 使用 buildFrame 构建 Modbus 帧
                if (!task.buildFrame) continue;

                AsyncRequest req;
                req.timeoutMs = task.timeoutMs;
                req.description = task.description;

                int buildRc = task.buildFrame(req.data, sizeof(req.data), &req.len);
                if (buildRc != 0) {
                    stats.failCount++;
                    continue;
                }

                // 捕获统计指针和回调
                TaskStats *statsPtr = &stats;
                auto processCb = task.processFunc;
                req.callback = [statsPtr, processCb](const uint8_t *resp, size_t resp_len, int rc) {
                    if (rc == 0) {
                        statsPtr->successCount++;
                    } else {
                        statsPtr->failCount++;
                    }
                    if (processCb) {
                        processCb(resp, resp_len, rc);
                    }
                };

                asyncBus_->submit(req);
                stats.pollCount++;
            }

            // 更新下次轮询时间
            nextPoll[gi] = now + std::chrono::milliseconds(group.pollIntervalMs);
        }

        // 短睡 10ms 再检查
        for (int i = 0; i < 10 && running_.load(); i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    printf("[PollingManager] 调度线程退出\n");
}

/* ============================================================
 * 统计和监控
 * ============================================================ */
std::map<std::string, PollingManager::TaskStatsSnapshot> PollingManager::getGroupStatsSnapshot() const {
    std::map<std::string, TaskStatsSnapshot> result;
    for (size_t i = 0; i < groups_.size() && i < groupStats_.size(); i++) {
        TaskStatsSnapshot snap;
        snap.pollCount    = groupStats_[i].pollCount.load();
        snap.successCount = groupStats_[i].successCount.load();
        snap.failCount    = groupStats_[i].failCount.load();
        result[groups_[i].name] = snap;
    }
    return result;
}

void PollingManager::printStatus() const {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║          异步轮询管理器状态报告                              ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    const auto &asyncStats = asyncBus_->getStats();
    printf("║ AsyncBus: 提交=%-8lu 成功=%-8lu 失败=%-8lu         ║\n",
           (unsigned long)asyncStats.totalSubmitted.load(),
           (unsigned long)asyncStats.totalSuccess.load(),
           (unsigned long)asyncStats.totalFailed.load());
    printf("║ 队列高水位: %-6lu  平均事务: %.1fms                      ║\n",
           (unsigned long)asyncStats.queueHighWaterMark.load(),
           asyncStats.avgTransactMs.load());

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ 任务组详情                                                   ║\n");

    for (size_t i = 0; i < groups_.size() && i < groupStats_.size(); i++) {
        const auto &g = groups_[i];
        const auto &s = groupStats_[i];
        double rate = s.pollCount > 0 ?
            (double)s.successCount.load() / s.pollCount.load() * 100.0 : 0.0;
        printf("║   [%zu] %-16s addr=0x%02X poll=%-6lu ok=%-6lu fail=%-4lu %.1f%%  ║\n",
               i, g.name.c_str(), g.devAddr,
               (unsigned long)s.pollCount.load(),
               (unsigned long)s.successCount.load(),
               (unsigned long)s.failCount.load(), rate);
    }

    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
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
