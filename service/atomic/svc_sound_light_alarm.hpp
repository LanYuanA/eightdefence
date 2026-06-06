/**
 * @file svc_sound_light_alarm.hpp
 * @brief 原子服务 - 声音报警
 */

#ifndef SVC_SOUND_LIGHT_ALARM_HPP
#define SVC_SOUND_LIGHT_ALARM_HPP

#include "svc_base.hpp"
#include <atomic>

class SvcSoundLightAlarm : public SvcBase {
public:
    std::string getName() const override { return "声音报警服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }

private:
    std::atomic<bool> active_{false};
};

#endif /* SVC_SOUND_LIGHT_ALARM_HPP */
