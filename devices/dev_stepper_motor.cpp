/**
 * @file dev_stepper_motor.cpp
 * @brief 一体式步进电机设备实现
 */

#include "dev_stepper_motor.hpp"
#include "core/device_config.h"
#include "service/modbus_service.hpp"
#include "core/logger.hpp"

#include <cstdio>
#include <cstring>
#include <functional>

// ---------- CANopen over Modbus 寄存器 ----------
#define REG_CTRL_WORD   0x6040  // 控制字
#define REG_OP_MODE     0x6060  // 运行模式
#define REG_ACCEL       0x6083  // 加速度 (32-bit)
#define REG_DECEL       0x6084  // 减速度 (32-bit)
#define REG_TARGET_SPEED 0x60FF // 目标速度 (32-bit, rpm)

void DevStepperMotor::init() {
    status_.reset();
    running_.store(false);
    speed_.store(0);
}

std::vector<DeviceTask> DevStepperMotor::getTasks() {
    (void)this;  // 步进电机无需周期轮询, 仅按需控制
    return {};
}

// ---------- 启动电机 ----------
bool DevStepperMotor::start(ModbusService& svc, int speedRpm) {
    uint8_t resp[512];
    size_t respLen = 0;
    int rc;

    // 1. 关机状态
    rc = svc.writeReg(addr_, REG_CTRL_WORD, 0x0006, resp, sizeof(resp), &respLen);
    if (rc != 0) { printf("  => [❌ %s] 关机失败 rc=%d\n", name_.c_str(), rc); return false; }

    // 2. 上电就绪
    rc = svc.writeReg(addr_, REG_CTRL_WORD, 0x0007, resp, sizeof(resp), &respLen);
    if (rc != 0) { printf("  => [❌ %s] 上电就绪失败 rc=%d\n", name_.c_str(), rc); return false; }

    // 3. 使能运行
    rc = svc.writeReg(addr_, REG_CTRL_WORD, 0x000F, resp, sizeof(resp), &respLen);
    if (rc != 0) { printf("  => [❌ %s] 使能失败 rc=%d\n", name_.c_str(), rc); return false; }

    // 4. 速度模式
    rc = svc.writeReg(addr_, REG_OP_MODE, 0x0003, resp, sizeof(resp), &respLen);
    if (rc != 0) { printf("  => [❌ %s] 设置速度模式失败 rc=%d\n", name_.c_str(), rc); return false; }

    // 5. 加速度 10 rps/s
    {
        uint16_t vals[2] = { 0x0000, 0x000A };  // 32-bit = 10
        rc = svc.writeMultiReg(addr_, REG_ACCEL, 2, vals, resp, sizeof(resp), &respLen);
    }
    if (rc != 0) { printf("  => [❌ %s] 设置加速度失败 rc=%d\n", name_.c_str(), rc); return false; }

    // 6. 减速度 10 rps/s
    {
        uint16_t vals[2] = { 0x0000, 0x000A };
        rc = svc.writeMultiReg(addr_, REG_DECEL, 2, vals, resp, sizeof(resp), &respLen);
    }
    if (rc != 0) { printf("  => [❌ %s] 设置减速度失败 rc=%d\n", name_.c_str(), rc); return false; }

    // 7. 目标速度
    {
        int rpm = (speedRpm > 0) ? speedRpm : 100;  // 默认100rpm
        uint16_t vals[2] = { 0x0000, static_cast<uint16_t>(rpm) };
        rc = svc.writeMultiReg(addr_, REG_TARGET_SPEED, 2, vals, resp, sizeof(resp), &respLen);
    }
    if (rc != 0) { printf("  => [❌ %s] 设置速度失败 rc=%d\n", name_.c_str(), rc); return false; }

    running_.store(true);
    speed_.store(speedRpm);
    status_.onSuccess();
    printf("  => [✅ %s] 电机已启动, 地址=0x%02X, 转速=%drpm\n", name_.c_str(), addr_, speedRpm);
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[%s] 电机启动, 地址=0x%02X, 转速=%drpm", name_.c_str(), addr_, speedRpm);
    return true;
}

// ---------- 停止电机 ----------
bool DevStepperMotor::stop(ModbusService& svc) {
    uint8_t resp[512];
    size_t respLen = 0;

    int rc = svc.writeReg(addr_, REG_CTRL_WORD, 0x0006, resp, sizeof(resp), &respLen);
    if (rc != 0) { printf("  => [❌ %s] 停止失败 rc=%d\n", name_.c_str(), rc); return false; }

    running_.store(false);
    speed_.store(0);
    printf("  => [🛑 %s] 电机已停止, 地址=0x%02X\n", name_.c_str(), addr_);
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[%s] 电机停止, 地址=0x%02X", name_.c_str(), addr_);
    return true;
}
