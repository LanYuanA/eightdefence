/**
 * @file svc_gas_response.hpp
 * @brief 原子服务 - 有害气体处理
 */

#ifndef SVC_GAS_RESPONSE_HPP
#define SVC_GAS_RESPONSE_HPP

#include "svc_base.hpp"
#include <atomic>
#include <mutex>

struct GasThresholds {
    int tvocHigh = 200;
    int ch2oHigh = 50;
    int o3High   = 60;
    int co2High  = 800;
};

class SvcGasResponse : public SvcBase {
public:
    std::string getName() const override { return "有害气体处理服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }

    void setThresholds(const GasThresholds& t);
    void checkAndControl(int tvoc, int ch2o, int o3, int co2);

private:
    std::atomic<bool> active_{false};
    std::mutex mtx_;
    GasThresholds thresholds_;
    bool purifierOn_ = false;
    bool purifyModeOn_ = false;

    void sendPurifierCommand(bool on);
    void sendPurifyModeCommand(bool on);
};

#endif /* SVC_GAS_RESPONSE_HPP */
