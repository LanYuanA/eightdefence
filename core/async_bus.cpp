/**
 * @file async_bus.cpp
 * @brief 异步串口总线实现
 */

#include "async_bus.hpp"
#include "serial_bus.hpp"
#include "service/modbus_service.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <unistd.h>

/* ============================================================
 * AsyncRequest 便捷方法
 * ============================================================ */
void AsyncRequest::buildReadReg(uint8_t devAddr, uint16_t regAddr, uint16_t count) {
    isWrite = false;
    ModbusService::buildReadRegFrame(devAddr, regAddr, count, data, sizeof(data), &len);
    char desc[64];
    snprintf(desc, sizeof(desc), "读寄存器 0x%02X:0x%04Xx%d", devAddr, regAddr, count);
    description = desc;
}

void AsyncRequest::buildWriteReg(uint8_t devAddr, uint16_t regAddr, uint16_t value) {
    isWrite = true;
    ModbusService::buildWriteRegFrame(devAddr, regAddr, value, data, sizeof(data), &len);
    char desc[64];
    snprintf(desc, sizeof(desc), "写寄存器 0x%02X:0x%04X=%u", devAddr, regAddr, value);
    description = desc;
}

void AsyncRequest::buildReadCoil(uint8_t devAddr, uint16_t coilAddr, uint16_t count) {
    isWrite = false;
    ModbusService::buildReadCoilFrame(devAddr, coilAddr, count, data, sizeof(data), &len);
    char desc[64];
    snprintf(desc, sizeof(desc), "读线圈 0x%02X:0x%04Xx%d", devAddr, coilAddr, count);
    description = desc;
}

void AsyncRequest::buildWriteCoil(uint8_t devAddr, uint16_t coilAddr, bool value) {
    isWrite = true;
    ModbusService::buildWriteCoilFrame(devAddr, coilAddr, value, data, sizeof(data), &len);
    char desc[64];
    snprintf(desc, sizeof(desc), "写线圈 0x%02X:0x%04X=%s", devAddr, coilAddr, value ? "ON" : "OFF");
    description = desc;
}

/* ============================================================
 * 构造 / 析构
 * ============================================================ */
AsyncBus::AsyncBus(SerialBus *bus) : bus_(bus) {}

AsyncBus::~AsyncBus() {
    stop();
}

/* ============================================================
 * 启动 / 停止
 * ============================================================ */
void AsyncBus::start() {
    if (running_.load()) return;
    running_ = true;
    ioThread_ = std::thread(&AsyncBus::ioThread, this);
    printf("[AsyncBus] I/O 线程已启动\n");
}

void AsyncBus::stop() {
    if (!running_.load()) return;
    running_ = false;
    queueCv_.notify_all();
    if (ioThread_.joinable()) {
        ioThread_.join();
    }
    printf("[AsyncBus] I/O 线程已停止\n");
}

/* ============================================================
 * 提交请求
 * ============================================================ */
void AsyncBus::submit(const AsyncRequest &req) {
    if (!running_.load()) {
        printf("[AsyncBus] 未运行, 丢弃请求: %s\n", req.description.c_str());
        return;
    }

    stats_.totalSubmitted++;
    if (req.isWrite) stats_.totalWriteOps++;

    {
        std::lock_guard<std::mutex> lock(queueMtx_);
        queue_.push(req);
        size_t qs = queue_.size();
        uint64_t hwm = stats_.queueHighWaterMark.load();
        while (qs > hwm) {
            if (stats_.queueHighWaterMark.compare_exchange_weak(hwm, qs)) break;
        }
    }
    queueCv_.notify_one();
}

size_t AsyncBus::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queueMtx_);
    return queue_.size();
}

/* ============================================================
 * I/O 线程主循环
 * ============================================================ */
void AsyncBus::ioThread() {
    printf("[AsyncBus] I/O 线程开始运行\n");

    while (running_.load()) {
        // 1. 从队列取请求 (带超时, 避免无法退出)
        AsyncRequest req;
        bool hasReq = false;

        {
            std::unique_lock<std::mutex> lock(queueMtx_);
            queueCv_.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                return !queue_.empty() || !running_.load();
            });

            if (!running_.load() && queue_.empty()) break;

            if (!queue_.empty()) {
                req = queue_.top();
                queue_.pop();
                hasReq = true;
            }
        }

        if (!hasReq) continue;

        // 2. 执行事务: 发送请求 + 接收响应
        uint8_t resp[256] = {0};
        size_t resp_len = 0;
        int rc = 0;
        bool success = false;

        for (int retry = 0; retry <= req.maxRetries; retry++) {
            if (!running_.load()) break;

            if (retry > 0) {
                stats_.totalRetries++;
                usleep(20000);  // 重试间隔 20ms
            }

            auto t_start = std::chrono::steady_clock::now();
            rc = bus_->transact(req.data, req.len, resp, sizeof(resp), &resp_len,
                                req.timeoutMs, req.isWrite);
            auto t_end = std::chrono::steady_clock::now();
            double transact_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

            // 更新平均事务时间
            double old_avg = stats_.avgTransactMs.load();
            uint64_t processed = stats_.totalProcessed.load();
            if (processed > 0) {
                stats_.avgTransactMs.store(old_avg * 0.9 + transact_ms * 0.1);
            } else {
                stats_.avgTransactMs.store(transact_ms);
            }

            if (rc == 0) {
                success = true;
                break;
            }
        }

        stats_.totalProcessed++;
        if (success) {
            stats_.totalSuccess++;
        } else {
            stats_.totalFailed++;
        }

        // 3. 调用回调
        if (req.callback) {
            req.callback(resp, resp_len, rc);
        }
    }

    printf("[AsyncBus] I/O 线程退出\n");
}
