/**
 * @file svc_drainage.cpp
 * @brief 排水控制服务实现（模拟）
 */

#include "svc_drainage.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcDrainage::activate() {
    if (active_.exchange(true)) return;

    // 模拟: 记录日志, 无真实硬件命令
    printf("  => [SvcDrainage] 排水泵启动指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
        "[排水控制服务] 排水泵启动指令已发送 (模拟模式, 未接入电机)");
}

void SvcDrainage::deactivate() {
    if (!active_.exchange(false)) return;

    printf("  => [SvcDrainage] 排水泵停止指令已发送 (模拟)\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[排水控制服务] 排水泵停止指令已发送 (模拟模式)");
}
