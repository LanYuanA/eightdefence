#ifndef SVC_FIRE_FAN_HPP
#define SVC_FIRE_FAN_HPP
#include "svc_base.hpp"
#include "devices/dev_stepper_motor.hpp"
#include <atomic>

class SvcFireFan : public SvcBase {
public:
    std::string getName() const override { return "排烟风机服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }
    void setMotor(DevStepperMotor* motor, int speedRpm = 300);
    int control(bool on);
    bool isOn() const { return on_.load(); }
private:
    std::atomic<bool> active_{false};
    std::atomic<bool> on_{false};
    DevStepperMotor* motor_ = nullptr;
    int speedRpm_ = 300;
};
#endif
