#ifndef MARINE_SAFETY_HPP
#define MARINE_SAFETY_HPP

#include "marine_types.hpp"

#include <cstdint>
#include <string>

namespace marine {

struct SafetyStatus { bool actuatorControlEnabled = true; bool unlocked = true; bool emergencyStopped = false; uint64_t unlockExpiresAt = 0; uint64_t remainingSeconds = 0; };

class MarineSafety {
public:
    explicit MarineSafety(std::string eventPath);
    SafetyStatus status(uint64_t nowMs) const;
    SafetyStatus unlock(const std::string& operatorName, uint64_t nowMs);
    SafetyStatus lock(const std::string& operatorName, uint64_t nowMs);
    bool mayControl(const Binding& binding, uint64_t nowMs);
    void emergencyStop(const std::string& operatorName, uint64_t nowMs);
    void resetEmergency(const std::string& operatorName, uint64_t nowMs);

private:
    std::string eventPath_;
    uint64_t unlockExpiresAt_ = 0;
    bool emergencyStopped_ = false;
    void audit(const std::string& type, const std::string& message, uint64_t nowMs) const;
};

} // namespace marine

#endif
