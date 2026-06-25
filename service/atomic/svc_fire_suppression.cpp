/**
 * @file svc_fire_suppression.cpp
 * @brief 灭火联动服务 - 控制水淋步进电机
 */

#include "svc_fire_suppression.hpp"
#include "core/logger.hpp"
#include "core/global_devices.hpp"
#include <cstdio>

void SvcFireSuppression::activate() {
    if (active_.exchange(true)) return;

    if (motor_ && g_modbus) {
        motor_->start(*g_modbus, 200);  // 水淋电机 200rpm
    } else {
        printf("  => [SvcFireSuppression] 灭火装置启动 (无电机连接, 模拟)\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[灭火联动服务] 灭火装置启动 (模拟模式)");
    }
}

void SvcFireSuppression::deactivate() {
    if (!active_.exchange(false)) return;

    if (motor_ && g_modbus) {
        motor_->stop(*g_modbus);
    } else {
        printf("  => [SvcFireSuppression] 灭火装置停止 (模拟)\n");
        Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
            "[灭火联动服务] 灭火装置停止 (模拟模式)");
    }
}
