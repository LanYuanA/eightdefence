#ifndef SVC_FIRE_CABIN_HPP
#define SVC_FIRE_CABIN_HPP
#include "svc_base.hpp"
#include "devices/dev_stepper_motor.hpp"
#include <atomic>

class SvcFireCabin : public SvcBase {
public:
    std::string getName() const override { return "舱门控制服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }
    void setMotor(DevStepperMotor* motor, int speedRpm = 200);
    int control(bool open);
    bool isOpen() const { return open_.load(); }
private:
    std::atomic<bool> active_{false};
    std::atomic<bool> open_{false};
    DevStepperMotor* motor_ = nullptr;
    int speedRpm_ = 200;
};
#endif
