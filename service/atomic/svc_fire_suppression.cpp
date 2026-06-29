/**
 * @file svc_fire_suppression.cpp
 * @brief 灭火联动服务实现（模拟）
 */

#include "svc_fire_suppression.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcFireSuppression::activate() {
    if (active_.exchange(true)) return;

    printf("  => [SvcFireSuppression] 灭火装置启动指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
        "[灭火联动服务] 灭火装置启动指令已发送 (模拟模式, 未接入灭火设备)");
}

void SvcFireSuppression::deactivate() {
    if (!active_.exchange(false)) return;

    printf("  => [SvcFireSuppression] 灭火装置停止指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[灭火联动服务] 灭火装置停止指令已发送 (模拟模式)");
}
