/**
 * @file svc_evacuation.hpp
 * @brief 原子服务 - 疏散引导
 *
 * 火灾时启动疏散广播和指示灯（模拟）
 */

#ifndef SVC_EVACUATION_HPP
#define SVC_EVACUATION_HPP

#include "svc_base.hpp"
#include "devices/dev_stepper_motor.hpp"
#include <atomic>

class SvcEvacuation : public SvcBase {
public:
    std::string getName() const override { return "疏散引导服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }
    void setMotor(DevStepperMotor* motor) { motor_ = motor; }

private:
    std::atomic<bool> active_{false};
    DevStepperMotor* motor_ = nullptr;
};

#endif /* SVC_EVACUATION_HPP */
