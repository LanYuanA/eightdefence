/**
 * @file svc_evacuation.cpp
 * @brief 疏散引导服务实现（模拟）
 */

#include "svc_evacuation.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcEvacuation::activate() {
    if (active_.exchange(true)) return;

    printf("  => [SvcEvacuation] 疏散广播和指示灯启动指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
        "[疏散引导服务] 疏散广播和应急指示灯启动指令已发送 (模拟模式)");
}

void SvcEvacuation::deactivate() {
    if (!active_.exchange(false)) return;

    printf("  => [SvcEvacuation] 疏散广播和指示灯停止指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[疏散引导服务] 疏散广播和应急指示灯停止指令已发送 (模拟模式)");
}
