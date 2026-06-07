/**
 * @file command_queue.cpp
 * @brief 异步指令下发队列实现
 */

#include "command_queue.hpp"
#include "async_bus.hpp"
#include "service/modbus_service.hpp"
#include <cstring>
#include <cstdio>
#include <chrono>

CommandQueue::CommandQueue(AsyncBus *asyncBus, ModbusService *svc)
    : asyncBus_(asyncBus), svc_(svc) {}

CommandQueue::~CommandQueue() { stop(); }

void CommandQueue::start() {
    running_ = true;
    printf("[CommandQueue] 已启动 (异步模式)\n");
}

void CommandQueue::stop() {
    running_ = false;
    resultsCv_.notify_all();
    printf("[CommandQueue] 已停止\n");
}

uint64_t CommandQueue::nextId() { return ++idCounter_; }

uint64_t CommandQueue::writeRegister(uint8_t devAddr, uint16_t regAddr, uint16_t value,
                                      CommandPriority priority,
                                      std::function<void(const CommandResult &)> callback) {
    Command cmd;
    cmd.id = nextId();
    cmd.type = CommandType::WRITE_REG;
    cmd.priority = priority;
    cmd.devAddr = devAddr;
    cmd.regAddr = regAddr;
    cmd.value = value;
    cmd.count = 1;
    cmd.timeoutMs = 2000;
    cmd.callback = callback;
    char desc[128];
    snprintf(desc, sizeof(desc), "写寄存器 0x%02X:0x%04X=%u", devAddr, regAddr, value);
    cmd.description = desc;
    return submit(cmd);
}

uint64_t CommandQueue::writeCoil(uint8_t devAddr, uint16_t coilAddr, bool value,
                                  CommandPriority priority,
                                  std::function<void(const CommandResult &)> callback) {
    Command cmd;
    cmd.id = nextId();
    cmd.type = CommandType::WRITE_COIL;
    cmd.priority = priority;
    cmd.devAddr = devAddr;
    cmd.regAddr = coilAddr;
    cmd.value = value ? 1 : 0;
    cmd.count = 1;
    cmd.timeoutMs = 2000;
    cmd.callback = callback;
    char desc[128];
    snprintf(desc, sizeof(desc), "写线圈 0x%02X:0x%04X=%s", devAddr, coilAddr, value ? "ON" : "OFF");
    cmd.description = desc;
    return submit(cmd);
}

uint64_t CommandQueue::submit(const Command &cmd) {
    if (!running_.load()) return 0;

    Command c = cmd;
    if (c.id == 0) c.id = nextId();
    stats_.totalSubmitted++;

    // 构建 Modbus 帧
    AsyncRequest req;
    req.isWrite = true;
    req.timeoutMs = c.timeoutMs;
    req.description = c.description;

    switch (c.type) {
        case CommandType::WRITE_REG:
            ModbusService::buildWriteRegFrame(c.devAddr, c.regAddr, c.value,
                                               req.data, sizeof(req.data), &req.len);
            break;
        case CommandType::WRITE_COIL:
            ModbusService::buildWriteCoilFrame(c.devAddr, c.regAddr, c.value != 0,
                                                req.data, sizeof(req.data), &req.len);
            break;
        default:
            return 0;
    }

    // 捕获需要的数据给回调
    uint64_t cmdId = c.id;
    auto cmdCb = c.callback;
    CommandQueue *self = this;
    auto startTime = std::make_shared<std::chrono::steady_clock::time_point>(
        std::chrono::steady_clock::now());

    req.callback = [self, cmdId, cmdCb, startTime](const uint8_t *resp, size_t resp_len, int rc) {
        auto endTime = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(endTime - *startTime).count();

        CommandResult result;
        result.execTimeMs = ms;
        if (rc == 0) {
            result.status = CommandStatus::SUCCESS;
            result.errorCode = 0;
            self->stats_.totalSuccess++;
        } else {
            result.status = CommandStatus::FAILED;
            result.errorCode = rc;
            self->stats_.totalFailed++;
        }
        if (resp_len > 0 && resp_len <= sizeof(result.responseData)) {
            memcpy(result.responseData, resp, resp_len);
            result.responseLen = resp_len;
        }

        self->storeResult(cmdId, result);
        if (cmdCb) cmdCb(result);
    };

    asyncBus_->submit(req);
    return c.id;
}

void CommandQueue::storeResult(uint64_t id, const CommandResult &result) {
    std::lock_guard<std::mutex> lock(resultsMtx_);
    results_[id] = result;
    resultsCv_.notify_all();
}

CommandResult CommandQueue::waitResult(uint64_t id, int timeoutMs) {
    std::unique_lock<std::mutex> lock(resultsMtx_);
    auto it = results_.find(id);
    if (it != results_.end()) return it->second;

    resultsCv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this, id]() {
        return results_.find(id) != results_.end();
    });

    it = results_.find(id);
    if (it != results_.end()) return it->second;

    CommandResult r;
    r.status = CommandStatus::TIMEOUT;
    r.errorMessage = "等待结果超时";
    return r;
}

bool CommandQueue::queryResult(uint64_t id, CommandResult &result) {
    std::lock_guard<std::mutex> lock(resultsMtx_);
    auto it = results_.find(id);
    if (it != results_.end()) {
        result = it->second;
        return true;
    }
    return false;
}

bool CommandQueue::cancel(uint64_t id) {
    std::lock_guard<std::mutex> lock(cancelledMtx_);
    cancelled_[id] = true;
    CommandResult r;
    r.status = CommandStatus::CANCELLED;
    storeResult(id, r);
    return true;
}
