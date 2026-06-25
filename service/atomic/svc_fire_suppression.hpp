/**
 * @file svc_fire_suppression.hpp
 * @brief 原子服务 - 灭火联动
 *
 * 火灾确认后启动灭火装置（模拟）
 */

#ifndef SVC_FIRE_SUPPRESSION_HPP
#define SVC_FIRE_SUPPRESSION_HPP

#include "svc_base.hpp"
#include "devices/dev_stepper_motor.hpp"
#include <atomic>

class SvcFireSuppression : public SvcBase {
public:
    std::string getName() const override { return "灭火联动服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }
    void setMotor(DevStepperMotor* motor) { motor_ = motor; }

private:
    std::atomic<bool> active_{false};
    DevStepperMotor* motor_ = nullptr;
};

#endif /* SVC_FIRE_SUPPRESSION_HPP */
