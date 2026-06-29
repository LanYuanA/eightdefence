/**
 * @file svc_air_quality_alert.hpp
 * @brief 原子服务 - 空气质量告警
 *
 * 监测 PM2.5/PM10 浓度，超阈值时联动净化设备
 */

#ifndef SVC_AIR_QUALITY_ALERT_HPP
#define SVC_AIR_QUALITY_ALERT_HPP

#include "svc_base.hpp"
#include <atomic>
#include <mutex>

struct AirQualityThresholds {
    float pm25Medium = 75.0f;    // PM2.5 中风险阈值 (μg/m³)
    float pm25High   = 150.0f;   // PM2.5 高风险阈值
    float pm10Medium = 150.0f;   // PM10 中风险阈值 (μg/m³)
    float pm10High   = 300.0f;   // PM10 高风险阈值
};

class SvcAirQualityAlert : public SvcBase {
public:
    std::string getName() const override { return "空气质量告警服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }

    void setThresholds(const AirQualityThresholds& t);
    void checkAndControl(float pm25, float pm10);

private:
    std::atomic<bool> active_{false};
    std::mutex mtx_;
    AirQualityThresholds thresholds_;
    bool purifierOn_ = false;

    void sendPurifierCommand(bool on);
};

#endif /* SVC_AIR_QUALITY_ALERT_HPP */
