/**
 * @file svc_sound_light_alarm.cpp
 * @brief 声光报警服务实现
 */

#include "svc_sound_light_alarm.hpp"
#include "core/global_devices.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcSoundLightAlarm::activate() {
    if (active_.exchange(true)) return;  // 已激活

    uint8_t resp[64];
    size_t resp_len = 0;
    int rc = dev_alarm.setAlarmOn(*g_modbus, resp, &resp_len);
    if (rc == 0) {
        printf("  => [SvcSoundLightAlarm] 声光报警器已激活\n");
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[声光报警服务] 声光报警器已激活");
    } else {
        printf("  => [SvcSoundLightAlarm] 声光报警器激活失败 (rc=%d)\n", rc);
        Logger::instance().log(LogLevel::ERROR_, __FILE__, __LINE__,
            "[声光报警服务] 声光报警器激活失败 (rc=%d)", rc);
    }
}

void SvcSoundLightAlarm::deactivate() {
    if (!active_.exchange(false)) return;  // 已关闭

    uint8_t resp[64];
    size_t resp_len = 0;
    int rc = dev_alarm.setAlarmOff(*g_modbus, resp, &resp_len);
    if (rc == 0) {
        printf("  => [SvcSoundLightAlarm] 声光报警器已关闭\n");
        Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
            "[声光报警服务] 声光报警器已关闭");
    } else {
        printf("  => [SvcSoundLightAlarm] 声光报警器关闭失败 (rc=%d)\n", rc);
    }
}
