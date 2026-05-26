/**
 * @file svc_temp_humidity_control.hpp
 * @brief 原子服务 - 温湿度调控
 */

#ifndef SVC_TEMP_HUMIDITY_CONTROL_HPP
#define SVC_TEMP_HUMIDITY_CONTROL_HPP

#include "svc_base.hpp"
#include <atomic>
#include <mutex>

class SvcTempHumidityControl : public SvcBase {
public:
    std::string getName() const override { return "温湿度调控服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }

    void setThresholds(float tempHigh, float tempLow, float humHigh, float humLow);
    void checkAndControl(float temperature, float humidity);

private:
    std::atomic<bool> active_{false};
    std::mutex mtx_;

    float tempHigh_ = 30.0f;
    float tempLow_  = 18.0f;
    float humHigh_  = 70.0f;
    float humLow_   = 30.0f;

    bool coolingOn_  = false;
    bool heatingOn_  = false;
    bool dehumidOn_  = false;
    bool humidifyOn_ = false;

    void sendCommand(const char* action, bool on);
};

#endif /* SVC_TEMP_HUMIDITY_CONTROL_HPP */
