/**
 * @file svc_ventilation.cpp
 * @brief 通风换气服务实现（模拟）
 */

#include "svc_ventilation.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcVentilation::activate() {
    if (active_.exchange(true)) return;

    printf("  => [SvcVentilation] 通风系统启动指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
        "[通风换气服务] 通风系统启动指令已发送 (模拟模式)");
}

void SvcVentilation::deactivate() {
    if (!active_.exchange(false)) return;

    printf("  => [SvcVentilation] 通风系统停止指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[通风换气服务] 通风系统停止指令已发送 (模拟模式)");
}
