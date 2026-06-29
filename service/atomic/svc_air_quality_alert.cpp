/**
 * @file svc_air_quality_alert.cpp
 * @brief 空气质量告警服务实现
 *
 * 设备离线时跳过指令，不占用总线时间。
 * 轮询系统会自动更新设备在线状态，设备恢复后下次 checkAndControl 自动重试。
 */

#include "svc_air_quality_alert.hpp"
#include "core/global_devices.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcAirQualityAlert::activate() {
    if (active_.exchange(true)) return;

    if (dev_purifier.isOnline()) {
        sendPurifierCommand(true);
    } else {
        printf("  => [SvcAirQualityAlert] 净化器离线, 跳过开启指令\n");
    }
    printf("  => [SvcAirQualityAlert] 空气质量告警服务已激活\n");
    Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
        "[空气质量告警服务] 已激活");
}

void SvcAirQualityAlert::deactivate() {
    if (!active_.exchange(false)) return;

    std::lock_guard<std::mutex> lock(mtx_);
    if (purifierOn_ && dev_purifier.isOnline()) {
        sendPurifierCommand(false);
    }
    purifierOn_ = false;
    printf("  => [SvcAirQualityAlert] 空气质量告警服务已停止\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[空气质量告警服务] 已停止");
}

void SvcAirQualityAlert::setThresholds(const AirQualityThresholds& t) {
    std::lock_guard<std::mutex> lock(mtx_);
    thresholds_ = t;
    printf("  => [SvcAirQualityAlert] 阈值已更新: PM2.5[%.0f/%.0f] PM10[%.0f/%.0f]\n",
           thresholds_.pm25Medium, thresholds_.pm25High,
           thresholds_.pm10Medium, thresholds_.pm10High);
}

void SvcAirQualityAlert::checkAndControl(float pm25, float pm10) {
    if (!active_.load()) return;

    std::lock_guard<std::mutex> lock(mtx_);

    bool exceeded = (pm25 > thresholds_.pm25Medium) || (pm10 > thresholds_.pm10Medium);
    bool purifierOnline = dev_purifier.isOnline();

    if (exceeded && !purifierOn_) {
        if (purifierOnline) {
            sendPurifierCommand(true);
            printf("  => [SvcAirQualityAlert] 空气质量超标, 开启净化器 (PM2.5:%.1f PM10:%.1f)\n",
                   pm25, pm10);
            Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
                "[空气质量告警] PM2.5:%.1f PM10:%.1f 超标, 开启净化器", pm25, pm10);
        } else {
            printf("  => [SvcAirQualityAlert] 净化器离线, 跳过开启指令 (PM2.5:%.1f PM10:%.1f)\n", pm25, pm10);
        }
    } else if (!exceeded && purifierOn_) {
        if (purifierOnline) {
            sendPurifierCommand(false);
            printf("  => [SvcAirQualityAlert] 空气质量恢复正常, 关闭净化器\n");
            Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
                "[空气质量告警] 空气质量恢复正常, 关闭净化器");
        } else {
            purifierOn_ = false;  // 离线时也清除状态
        }
    }
}

void SvcAirQualityAlert::sendPurifierCommand(bool on) {
    uint8_t resp[64];
    size_t resp_len = 0;
    dev_purifier.setPower(*g_modbus, on ? 1 : 0, resp, &resp_len);
    purifierOn_ = on;
}
