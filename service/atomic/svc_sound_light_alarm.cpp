/**
 * @file svc_sound_light_alarm.cpp
 * @brief 声音报警服务实现
 *
 * 设备离线时跳过指令，不占用总线时间。
 */

#include "svc_sound_light_alarm.hpp"
#include "core/global_devices.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcSoundLightAlarm::activate() {
    if (active_.exchange(true)) return;  // 已激活

    if (!dev_alarm.isOnline()) {
        printf("  => [SvcSoundLightAlarm] 报警器离线, 跳过激活指令\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[声音报警服务] 报警器离线, 跳过激活指令");
        return;
    }

    uint8_t resp[64];
    size_t resp_len = 0;
    int rc = dev_alarm.setAlarmOn(*g_modbus, resp, &resp_len);
    if (rc == 0) {
        printf("  => [SvcSoundLightAlarm] 声音报警器已激活\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[声音报警服务] 声音报警器已激活");
    } else {
        printf("  => [SvcSoundLightAlarm] 声音报警器激活失败 (rc=%d)\n", rc);
        Logger::instance().log(LogLevel::ERROR_, __FILE__, __LINE__,
            "[声音报警服务] 声音报警器激活失败 (rc=%d)", rc);
    }
}

void SvcSoundLightAlarm::deactivate() {
    if (!active_.exchange(false)) return;  // 已关闭

    if (!dev_alarm.isOnline()) {
        printf("  => [SvcSoundLightAlarm] 报警器离线, 跳过关闭指令\n");
        return;
    }

    uint8_t resp[64];
    size_t resp_len = 0;
    int rc = dev_alarm.setAlarmOff(*g_modbus, resp, &resp_len);
    if (rc == 0) {
        printf("  => [SvcSoundLightAlarm] 声音报警器已关闭\n");
        Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
            "[声音报警服务] 声音报警器已关闭");
    } else {
        printf("  => [SvcSoundLightAlarm] 声音报警器关闭失败 (rc=%d)\n", rc);
    }
}
