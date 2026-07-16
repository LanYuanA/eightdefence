/**
 * @file svc_sound_light_alarm.cpp
 * @brief 声音报警服务实现
 */

#include "svc_sound_light_alarm.hpp"
#include "core/global_devices.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcSoundLightAlarm::activate() {
    if (active_.exchange(true)) { printf("  => [SvcSoundLightAlarm] 已处于激活状态, 跳过\n"); return; }
    if (!g_modbus) { printf("  => [SvcSoundLightAlarm] g_modbus为空, 无法激活!\n"); active_.store(false); return; }
    uint8_t resp[64];
    size_t resp_len = 0;
    int rc = dev_alarm.setAlarmOn(*g_modbus, resp, &resp_len);
    if (rc == 0) {
        printf("  => [SvcSoundLightAlarm] 声音报警器已激活\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[声音报警服务] 声音报警器已激活");
    } else {
        printf("  => [SvcSoundLightAlarm] 声音报警器激活失败 (rc=%d)\n", rc);
        active_.store(false);
        Logger::instance().log(LogLevel::ERROR_, __FILE__, __LINE__,
            "[声音报警服务] 声音报警器激活失败 (rc=%d)", rc);
    }
}

void SvcSoundLightAlarm::deactivate() {
    if (!active_.exchange(false)) { printf("  => [SvcSoundLightAlarm] 已处于关闭状态, 跳过\n"); return; }
    if (!g_modbus) { printf("  => [SvcSoundLightAlarm] g_modbus为空, 无法关闭!\n"); return; }
    uint8_t resp[64];
    size_t resp_len = 0;
    int rc = dev_alarm.setAlarmOff(*g_modbus, resp, &resp_len);
    if (rc == 0) {
        printf("  => [SvcSoundLightAlarm] 声音报警器已关闭\n");
        Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
            "[声音报警服务] 声音报警器已关闭");
    } else {
        printf("  => [SvcSoundLightAlarm] 声音报警器关闭失败 (rc=%d)\n", rc);
        active_.store(true);
    }
}
