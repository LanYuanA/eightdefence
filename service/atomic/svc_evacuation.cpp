/**
 * @file svc_evacuation.cpp
 * @brief 疏散引导服务 - 控制排烟风机步进电机
 */

#include "svc_evacuation.hpp"
#include "core/logger.hpp"
#include "core/global_devices.hpp"
#include <cstdio>

void SvcEvacuation::activate() {
    if (active_.exchange(true)) return;

    if (motor_ && g_modbus) {
        motor_->start(*g_modbus, 300);  // 排烟风机 300rpm
    } else {
        printf("  => [SvcEvacuation] 排烟风机启动 (模拟)\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[疏散引导服务] 排烟风机启动 (模拟模式)");
    }
}

void SvcEvacuation::deactivate() {
    if (!active_.exchange(false)) return;

    if (motor_ && g_modbus) {
        motor_->stop(*g_modbus);
    } else {
        printf("  => [SvcEvacuation] 排烟风机停止 (模拟)\n");
        Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
            "[疏散引导服务] 排烟风机停止 (模拟模式)");
    }
}
