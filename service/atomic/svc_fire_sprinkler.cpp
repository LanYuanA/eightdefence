#include "svc_fire_sprinkler.hpp"
#include "core/logger.hpp"
#include "core/global_devices.hpp"
#include <cstdio>

void SvcFireSprinkler::setMotor(DevStepperMotor* motor, int speedRpm) {
    motor_ = motor;
    speedRpm_ = speedRpm;
}

void SvcFireSprinkler::activate() {
    if (active_.exchange(true)) return;
    if (motor_ && g_modbus) {
        motor_->start(*g_modbus, speedRpm_);
    } else {
        printf("  => [SvcFireSprinkler] 水淋控制已激活 (无电机连接, 模拟)\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__, "[水淋] 已激活 (模拟)");
    }
}

void SvcFireSprinkler::deactivate() {
    if (!active_.exchange(false)) return;
    if (motor_ && g_modbus) {
        motor_->stop(*g_modbus);
    }
    on_.store(false);
}

int SvcFireSprinkler::control(bool on) {
    on_.store(on);
    if (!motor_ || !g_modbus) {
        printf("  => [SvcFireSprinkler] 水淋: %s (无电机, 模拟)\n", on ? "开启" : "关闭");
        return 0;
    }
    if (on) {
        motor_->start(*g_modbus, speedRpm_);
        active_.store(true);
    } else {
        motor_->stop(*g_modbus);
        active_.store(false);
    }
    printf("  => [SvcFireSprinkler] 水淋: %s\n", on ? "开启" : "关闭");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__, "[水淋] 电机 %s", on ? "开启" : "关闭");
    return 0;
}
