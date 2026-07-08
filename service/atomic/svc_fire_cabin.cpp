#include "svc_fire_cabin.hpp"
#include "core/logger.hpp"
#include "core/global_devices.hpp"
#include <cstdio>

void SvcFireCabin::setMotor(DevStepperMotor* motor, int speedRpm) {
    motor_ = motor;
    speedRpm_ = speedRpm;
}

void SvcFireCabin::activate() {
    if (active_.exchange(true)) return;
    if (motor_ && g_modbus) {
        motor_->start(*g_modbus, speedRpm_);
    } else {
        printf("  => [SvcFireCabin] 舱门控制已激活 (无电机连接, 模拟)\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__, "[舱门] 已激活 (模拟)");
    }
}

void SvcFireCabin::deactivate() {
    if (!active_.exchange(false)) return;
    if (motor_ && g_modbus) {
        motor_->stop(*g_modbus);
    }
    open_.store(false);
}

int SvcFireCabin::control(bool open) {
    open_.store(open);
    if (!motor_ || !g_modbus) {
        printf("  => [SvcFireCabin] 舱门: %s (无电机, 模拟)\n", open ? "开启" : "关闭");
        return 0;
    }
    if (open) {
        motor_->start(*g_modbus, speedRpm_);
        active_.store(true);
    } else {
        motor_->stop(*g_modbus);
        active_.store(false);
    }
    printf("  => [SvcFireCabin] 舱门: %s\n", open ? "开启" : "关闭");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__, "[舱门] 电机 %s", open ? "开启" : "关闭");
    return 0;
}
