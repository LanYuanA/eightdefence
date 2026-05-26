/**
 * @file svc_temp_humidity_control.cpp
 * @brief 温湿度调控服务实现
 */

#include "svc_temp_humidity_control.hpp"
#include "core/global_devices.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcTempHumidityControl::activate() {
    active_.store(true);
    printf("  => [SvcTempHumidityControl] 温湿度调控服务已激活\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[温湿度调控服务] 服务已激活, 等待阈值检测");
}

void SvcTempHumidityControl::deactivate() {
    if (!active_.exchange(false)) return;

    std::lock_guard<std::mutex> lock(mtx_);
    uint8_t resp[64];
    size_t resp_len = 0;

    if (coolingOn_)  { dev_ac.setCoolOff(*g_modbus, resp, &resp_len); coolingOn_ = false; }
    if (heatingOn_)  { dev_ac.setHeatOff(*g_modbus, resp, &resp_len); heatingOn_ = false; }
    if (dehumidOn_)  { dev_humidifier.setDehumidify(*g_modbus, 0, resp, &resp_len); dehumidOn_ = false; }
    if (humidifyOn_) { dev_humidifier.setHumidify(*g_modbus, 0, resp, &resp_len); humidifyOn_ = false; }

    printf("  => [SvcTempHumidityControl] 温湿度调控服务已停止, 所有设备已关闭\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[温湿度调控服务] 服务已停止");
}

void SvcTempHumidityControl::setThresholds(float tempHigh, float tempLow, float humHigh, float humLow) {
    std::lock_guard<std::mutex> lock(mtx_);
    tempHigh_ = tempHigh;
    tempLow_  = tempLow;
    humHigh_  = humHigh;
    humLow_   = humLow;
    printf("  => [SvcTempHumidityControl] 阈值已更新: 温度[%.1f~%.1f] 湿度[%.1f~%.1f]\n",
           tempLow_, tempHigh_, humLow_, humHigh_);
}

void SvcTempHumidityControl::checkAndControl(float temperature, float humidity) {
    if (!active_.load()) return;

    std::lock_guard<std::mutex> lock(mtx_);
    uint8_t resp[64];
    size_t resp_len = 0;
    bool changed = false;

    // 温度控制
    if (temperature > tempHigh_ && !coolingOn_) {
        dev_ac.setCoolOn(*g_modbus, resp, &resp_len);
        coolingOn_ = true;
        if (heatingOn_) { dev_ac.setHeatOff(*g_modbus, resp, &resp_len); heatingOn_ = false; }
        printf("  => [SvcTempHumidityControl] 温度 %.1f°C > %.1f°C, 开启制冷\n", temperature, tempHigh_);
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[温湿度调控] 温度 %.1f°C 超过阈值 %.1f°C, 开启空调制冷", temperature, tempHigh_);
        changed = true;
    } else if (temperature < tempLow_ && !heatingOn_) {
        dev_ac.setHeatOn(*g_modbus, resp, &resp_len);
        heatingOn_ = true;
        if (coolingOn_) { dev_ac.setCoolOff(*g_modbus, resp, &resp_len); coolingOn_ = false; }
        printf("  => [SvcTempHumidityControl] 温度 %.1f°C < %.1f°C, 开启制热\n", temperature, tempLow_);
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[温湿度调控] 温度 %.1f°C 低于阈值 %.1f°C, 开启空调制热", temperature, tempLow_);
        changed = true;
    } else if (temperature >= tempLow_ && temperature <= tempHigh_) {
        if (coolingOn_) { dev_ac.setCoolOff(*g_modbus, resp, &resp_len); coolingOn_ = false; changed = true; }
        if (heatingOn_) { dev_ac.setHeatOff(*g_modbus, resp, &resp_len); heatingOn_ = false; changed = true; }
    }

    // 湿度控制
    if (humidity > humHigh_ && !dehumidOn_) {
        dev_humidifier.setDehumidify(*g_modbus, 1, resp, &resp_len);
        dehumidOn_ = true;
        if (humidifyOn_) { dev_humidifier.setHumidify(*g_modbus, 0, resp, &resp_len); humidifyOn_ = false; }
        printf("  => [SvcTempHumidityControl] 湿度 %.1f%% > %.1f%%, 开启除湿\n", humidity, humHigh_);
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[温湿度调控] 湿度 %.1f%% 超过阈值 %.1f%%, 开启除湿", humidity, humHigh_);
        changed = true;
    } else if (humidity < humLow_ && !humidifyOn_) {
        dev_humidifier.setHumidify(*g_modbus, 1, resp, &resp_len);
        humidifyOn_ = true;
        if (dehumidOn_) { dev_humidifier.setDehumidify(*g_modbus, 0, resp, &resp_len); dehumidOn_ = false; }
        printf("  => [SvcTempHumidityControl] 湿度 %.1f%% < %.1f%%, 开启加湿\n", humidity, humLow_);
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[温湿度调控] 湿度 %.1f%% 低于阈值 %.1f%%, 开启加湿", humidity, humLow_);
        changed = true;
    } else if (humidity >= humLow_ && humidity <= humHigh_) {
        if (dehumidOn_)  { dev_humidifier.setDehumidify(*g_modbus, 0, resp, &resp_len); dehumidOn_ = false; changed = true; }
        if (humidifyOn_) { dev_humidifier.setHumidify(*g_modbus, 0, resp, &resp_len); humidifyOn_ = false; changed = true; }
    }

    if (changed) {
        printf("  => [SvcTempHumidityControl] 当前状态: 制冷=%s 制热=%s 除湿=%s 加湿=%s\n",
               coolingOn_ ? "开" : "关", heatingOn_ ? "开" : "关",
               dehumidOn_ ? "开" : "关", humidifyOn_ ? "开" : "关");
    }
}
