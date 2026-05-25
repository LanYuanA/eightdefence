/**
 * @file command_queue.cpp
 * @brief 指令下发队列实现
 */

#include "command_queue.hpp"
#include "logger.hpp"
#include "serial_bus.hpp"
#include "service/modbus_service.hpp"
#include <cstring>
#include <algorithm>
#include <unistd.h>

/* ============================================================
 * 构造 / 析构
 * ============================================================ */
CommandQueue::CommandQueue(SerialBus *bus, ModbusService *svc, int workerCount)
    : bus_(bus), svc_(svc), workerCount_(workerCount) {}

CommandQueue::~CommandQueue() {
    stop();
}

/* ============================================================
 * 生成唯一ID
 * ============================================================ */
uint64_t CommandQueue::nextId() {
    return ++idCounter_;
}

/* ============================================================
 * 启动
 * ============================================================ */
void CommandQueue::start() {
    if (running_.load()) return;

    running_ = true;
    LOG_INFO("命令队列启动, 工作线程数: %d", workerCount_);

    for (int i = 0; i < workerCount_; i++) {
        workers_.emplace_back(&CommandQueue::workerThread, this, i);
    }
}

/* ============================================================
 * 停止
 * ============================================================ */
void CommandQueue::stop() {
    if (!running_.load()) return;

    running_ = false;
    queueCv_.notify_all();

    for (auto &t : workers_) {
        if (t.joinable()) t.join();
    }
    workers_.clear();

    LOG_INFO("命令队列已停止");
}

/* ============================================================
 * 提交写寄存器命令
 * ============================================================ */
uint64_t CommandQueue::writeRegister(uint8_t devAddr, uint16_t regAddr, uint16_t value,
                                      CommandPriority priority,
                                      std::function<void(const CommandResult &)> callback) {
    Command cmd;
    cmd.id          = nextId();
    cmd.type        = CommandType::WRITE_REG;
    cmd.priority    = priority;
    cmd.devAddr     = devAddr;
    cmd.regAddr     = regAddr;
    cmd.value       = value;
    cmd.count       = 1;
    cmd.timeoutMs   = 2000;
    cmd.maxRetries  = 2;
    cmd.callback    = callback;

    char desc[128];
    snprintf(desc, sizeof(desc), "写寄存器 Addr=0x%02X Reg=0x%04X Val=%u",
             devAddr, regAddr, value);
    cmd.description = desc;

    return submit(cmd);
}

/* ============================================================
 * 提交写线圈命令
 * ============================================================ */
uint64_t CommandQueue::writeCoil(uint8_t devAddr, uint16_t coilAddr, bool value,
                                  CommandPriority priority,
                                  std::function<void(const CommandResult &)> callback) {
    Command cmd;
    cmd.id          = nextId();
    cmd.type        = CommandType::WRITE_COIL;
    cmd.priority    = priority;
    cmd.devAddr     = devAddr;
    cmd.regAddr     = coilAddr;
    cmd.value       = value ? 1 : 0;
    cmd.count       = 1;
    cmd.timeoutMs   = 2000;
    cmd.maxRetries  = 2;
    cmd.callback    = callback;

    char desc[128];
    snprintf(desc, sizeof(desc), "写线圈 Addr=0x%02X Coil=0x%04X Val=%s",
             devAddr, coilAddr, value ? "ON" : "OFF");
    cmd.description = desc;

    return submit(cmd);
}

/* ============================================================
 * 提交通用命令
 * ============================================================ */
uint64_t CommandQueue::submit(const Command &cmd) {
    if (!running_.load()) {
        LOG_WARNING("命令队列未运行, 丢弃命令: %s", cmd.description.c_str());
        return 0;
    }

    Command c = cmd;
    if (c.id == 0) c.id = nextId();

    stats_.totalSubmitted++;

    {
        std::lock_guard<std::mutex> lock(queueMtx_);
        queue_.push(c);
        stats_.currentQueueSize = queue_.size();
    }
    queueCv_.notify_one();

    LOG_INFO("命令已提交 [ID=%lu] %s (优先级=%d)",
             (unsigned long)c.id, c.description.c_str(), (int)c.priority);

    return c.id;
}

/* ============================================================
 * 查询结果 (阻塞)
 * ============================================================ */
CommandResult CommandQueue::waitResult(uint64_t id, int timeoutMs) {
    std::unique_lock<std::mutex> lock(resultsMtx_);

    /* 先检查是否已有结果 */
    auto it = results_.find(id);
    if (it != results_.end()) {
        return it->second;
    }

    /* 等待结果 */
    resultsCv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this, id]() {
        return results_.find(id) != results_.end();
    });

    it = results_.find(id);
    if (it != results_.end()) {
        return it->second;
    }

    /* 超时 */
    CommandResult r;
    r.status = CommandStatus::TIMEOUT;
    r.errorMessage = "等待结果超时";
    return r;
}

/* ============================================================
 * 查询结果 (非阻塞)
 * ============================================================ */
bool CommandQueue::queryResult(uint64_t id, CommandResult &result) {
    std::lock_guard<std::mutex> lock(resultsMtx_);
    auto it = results_.find(id);
    if (it != results_.end()) {
        result = it->second;
        return true;
    }
    return false;
}

/* ============================================================
 * 取消命令
 * ============================================================ */
bool CommandQueue::cancel(uint64_t id) {
    std::lock_guard<std::mutex> lock(cancelledMtx_);
    cancelled_[id] = true;

    /* 存储取消结果 */
    {
        std::lock_guard<std::mutex> rlock(resultsMtx_);
        CommandResult r;
        r.status = CommandStatus::CANCELLED;
        r.errorMessage = "用户取消";
        results_[id] = r;
    }
    resultsCv_.notify_all();

    LOG_INFO("命令已取消 [ID=%lu]", (unsigned long)id);
    return true;
}

/* ============================================================
 * 清空队列
 * ============================================================ */
void CommandQueue::clear() {
    std::lock_guard<std::mutex> lock(queueMtx_);
    while (!queue_.empty()) {
        queue_.pop();
    }
    stats_.currentQueueSize = 0;
    LOG_INFO("命令队列已清空");
}

/* ============================================================
 * 队列大小
 * ============================================================ */
size_t CommandQueue::size() const {
    std::lock_guard<std::mutex> lock(queueMtx_);
    return queue_.size();
}

/* ============================================================
 * 工作线程
 * ============================================================ */
void CommandQueue::workerThread(int threadId) {
    LOG_INFO("命令处理线程 #%d 已启动", threadId);

    while (running_.load()) {
        Command cmd;
        bool hasCmd = false;

        /* 从队列取命令 (带超时, 避免无法退出) */
        {
            std::unique_lock<std::mutex> lock(queueMtx_);
            queueCv_.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                return !queue_.empty() || !running_.load();
            });

            if (!running_.load() && queue_.empty()) break;

            if (!queue_.empty()) {
                cmd = queue_.top();
                queue_.pop();
                stats_.currentQueueSize = queue_.size();
                hasCmd = true;
            }
        }

        if (!hasCmd) continue;

        /* 检查是否已取消 */
        {
            std::lock_guard<std::mutex> lock(cancelledMtx_);
            if (cancelled_.count(cmd.id)) {
                cancelled_.erase(cmd.id);
                continue;  // 结果已在 cancel() 中存储
            }
        }

        /* 执行命令 (支持重试) */
        cmd.result.status = CommandStatus::EXECUTING;
        bool success = false;

        for (int retry = 0; retry <= cmd.maxRetries; retry++) {
            if (!running_.load()) break;

            if (retry > 0) {
                LOG_WARNING("命令 [ID=%lu] 重试第 %d 次: %s",
                            (unsigned long)cmd.id, retry, cmd.description.c_str());
                usleep(50000);  // 重试间隔 50ms
            }

            success = executeCommand(cmd);
            if (success) break;
        }

        /* 存储结果 */
        if (success) {
            cmd.result.status = CommandStatus::SUCCESS;
            stats_.totalSuccess++;
        } else {
            cmd.result.status = CommandStatus::FAILED;
            stats_.totalFailed++;
        }
        stats_.totalExecuted++;

        /* 更新平均执行时间 */
        double curAvg = stats_.avgExecTimeMs.load();
        uint64_t executed = stats_.totalExecuted.load();
        stats_.avgExecTimeMs = (curAvg * (executed - 1) + cmd.result.execTimeMs) / executed;

        /* 存入结果表 */
        {
            std::lock_guard<std::mutex> lock(resultsMtx_);
            results_[cmd.id] = cmd.result;
        }
        resultsCv_.notify_all();

        /* 调用回调 */
        if (cmd.callback) {
            cmd.callback(cmd.result);
        }
    }

    LOG_INFO("命令处理线程 #%d 已退出", threadId);
}

/* ============================================================
 * 执行单个命令
 * ============================================================ */
bool CommandQueue::executeCommand(Command &cmd) {
    if (!svc_) {
        cmd.result.errorCode = -1;
        cmd.result.errorMessage = "ModbusService 未初始化";
        return false;
    }

    auto startTime = std::chrono::steady_clock::now();

    uint8_t resp[256];
    size_t respLen = 0;
    int rc = 0;

    switch (cmd.type) {
        case CommandType::WRITE_REG:
            rc = svc_->writeReg(cmd.devAddr, cmd.regAddr, cmd.value,
                                resp, sizeof(resp), &respLen);
            break;

        case CommandType::WRITE_COIL:
            rc = svc_->writeCoil(cmd.devAddr, cmd.regAddr, cmd.value != 0,
                                 resp, sizeof(resp), &respLen);
            break;

        case CommandType::READ_REG:
            rc = svc_->readReg(cmd.devAddr, cmd.regAddr, cmd.count,
                               resp, sizeof(resp), &respLen);
            break;

        case CommandType::READ_COIL:
            rc = svc_->readCoil(cmd.devAddr, cmd.regAddr, cmd.count,
                                resp, sizeof(resp), &respLen);
            break;

        default:
            cmd.result.errorCode = -2;
            cmd.result.errorMessage = "未知命令类型";
            return false;
    }

    auto endTime = std::chrono::steady_clock::now();
    cmd.result.execTimeMs = std::chrono::duration<double, std::milli>(
        endTime - startTime).count();

    /* 存储响应数据 */
    if (respLen > 0 && respLen <= sizeof(cmd.result.responseData)) {
        memcpy(cmd.result.responseData, resp, respLen);
        cmd.result.responseLen = respLen;
    }

    if (rc == 0) {
        cmd.result.errorCode = 0;
        LOG_INFO("命令执行成功 [ID=%lu] %s (%.1fms)",
                 (unsigned long)cmd.id, cmd.description.c_str(),
                 cmd.result.execTimeMs);
        return true;
    } else {
        cmd.result.errorCode = rc;
        char err[256];
        snprintf(err, sizeof(err), "Modbus 通信错误: rc=%d", rc);
        cmd.result.errorMessage = err;
        LOG_WARNING("命令执行失败 [ID=%lu] %s (rc=%d, %.1fms)",
                    (unsigned long)cmd.id, cmd.description.c_str(),
                    rc, cmd.result.execTimeMs);
        return false;
    }
}