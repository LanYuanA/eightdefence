/**
 * @file svc_ventilation.hpp
 * @brief 原子服务 - 通风换气
 *
 * 环境指标综合超标时启动通风（模拟）
 */

#ifndef SVC_VENTILATION_HPP
#define SVC_VENTILATION_HPP

#include "svc_base.hpp"
#include <atomic>

class SvcVentilation : public SvcBase {
public:
    std::string getName() const override { return "通风换气服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }

private:
    std::atomic<bool> active_{false};
};

#endif /* SVC_VENTILATION_HPP */
