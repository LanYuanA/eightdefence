#include "svc_fire_fan.hpp"
#include "core/logger.hpp"
#include "core/global_devices.hpp"
#include <cstdio>

void SvcFireFan::setMotor(DevStepperMotor* motor, int speedRpm) {
    motor_ = motor;
    speedRpm_ = speedRpm;
}

void SvcFireFan::activate() {
    if (active_.exchange(true)) return;
    if (motor_ && g_modbus) {
        motor_->start(*g_modbus, speedRpm_);
    } else {
        printf("  => [SvcFireFan] 排烟风机已激活 (无电机连接, 模拟)\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__, "[排烟] 已激活 (模拟)");
    }
}

void SvcFireFan::deactivate() {
    if (!active_.exchange(false)) return;
    if (motor_ && g_modbus) {
        motor_->stop(*g_modbus);
    }
    on_.store(false);
}

int SvcFireFan::control(bool on) {
    on_.store(on);
    if (!motor_ || !g_modbus) {
        printf("  => [SvcFireFan] 排烟: %s (无电机, 模拟)\n", on ? "开启" : "关闭");
        return 0;
    }
    if (on) {
        motor_->start(*g_modbus, speedRpm_);
        active_.store(true);
    } else {
        motor_->stop(*g_modbus);
        active_.store(false);
    }
    printf("  => [SvcFireFan] 排烟: %s\n", on ? "开启" : "关闭");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__, "[排烟] 电机 %s", on ? "开启" : "关闭");
    return 0;
}
