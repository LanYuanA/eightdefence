/**
 * @file svc_gas_response.cpp
 * @brief 有害气体处理服务实现
 */

#include "svc_gas_response.hpp"
#include "core/global_devices.hpp"
#include "core/logger.hpp"
#include <cstdio>

void SvcGasResponse::activate() {
    if (active_.exchange(true)) return;

    sendPurifierCommand(true);
    sendPurifyModeCommand(true);
    printf("  => [SvcGasResponse] 有害气体处理服务已激活: 净化器开启\n");
    Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
        "[有害气体处理服务] 已激活: 净化器和净化模式已开启");
}

void SvcGasResponse::deactivate() {
    if (!active_.exchange(false)) return;

    std::lock_guard<std::mutex> lock(mtx_);
    uint8_t resp[64];
    size_t resp_len = 0;

    if (purifyModeOn_) { dev_humidifier.setPurify(*g_modbus, 0, resp, &resp_len); purifyModeOn_ = false; }
    if (purifierOn_)   { dev_purifier.setPower(*g_modbus, 0, resp, &resp_len); purifierOn_ = false; }

    printf("  => [SvcGasResponse] 有害气体处理服务已停止\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[有害气体处理服务] 已停止, 净化设备已关闭");
}

void SvcGasResponse::setThresholds(const GasThresholds& t) {
    std::lock_guard<std::mutex> lock(mtx_);
    thresholds_ = t;
    printf("  => [SvcGasResponse] 阈值已更新: TVOC>%d CH2O>%d O3>%d CO2>%d\n",
           thresholds_.tvocHigh, thresholds_.ch2oHigh, thresholds_.o3High, thresholds_.co2High);
}

void SvcGasResponse::checkAndControl(int tvoc, int ch2o, int o3, int co2) {
    if (!active_.load()) return;

    std::lock_guard<std::mutex> lock(mtx_);
    uint8_t resp[64];
    size_t resp_len = 0;

    bool exceeded = (tvoc > thresholds_.tvocHigh) ||
                    (ch2o > thresholds_.ch2oHigh) ||
                    (o3   > thresholds_.o3High)   ||
                    (co2  > thresholds_.co2High);

    if (exceeded && !purifierOn_) {
        sendPurifierCommand(true);
        sendPurifyModeCommand(true);
        printf("  => [SvcGasResponse] 气体超标, 开启净化器 (TVOC:%d CH2O:%d O3:%d CO2:%d)\n",
               tvoc, ch2o, o3, co2);
        Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__,
            "[有害气体处理] 气体超标, 开启净化器 (TVOC:%d CH2O:%d O3:%d CO2:%d)",
            tvoc, ch2o, o3, co2);
    } else if (!exceeded && purifierOn_) {
        if (purifyModeOn_) { dev_humidifier.setPurify(*g_modbus, 0, resp, &resp_len); purifyModeOn_ = false; }
        dev_purifier.setPower(*g_modbus, 0, resp, &resp_len);
        purifierOn_ = false;
        printf("  => [SvcGasResponse] 气体恢复正常, 关闭净化器\n");
        Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
            "[有害气体处理] 气体浓度恢复正常, 关闭净化器");
    }
}

void SvcGasResponse::sendPurifierCommand(bool on) {
    uint8_t resp[64];
    size_t resp_len = 0;
    dev_purifier.setPower(*g_modbus, on ? 1 : 0, resp, &resp_len);
    purifierOn_ = on;
}

void SvcGasResponse::sendPurifyModeCommand(bool on) {
    uint8_t resp[64];
    size_t resp_len = 0;
    dev_humidifier.setPurify(*g_modbus, on ? 1 : 0, resp, &resp_len);
    purifyModeOn_ = on;
}
