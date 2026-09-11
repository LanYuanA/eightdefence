#include "marine_safety.hpp"

#include "json_value.hpp"

#include <fstream>

namespace marine {
MarineSafety::MarineSafety(std::string eventPath) : eventPath_(std::move(eventPath)) {}
SafetyStatus MarineSafety::status(uint64_t nowMs) const {
    SafetyStatus current;
    current.emergencyStopped = emergencyStopped_;
    current.actuatorControlEnabled = !emergencyStopped_;
    current.unlocked = !emergencyStopped_;
    current.unlockExpiresAt = unlockExpiresAt_;
    current.remainingSeconds = 0;
    return current;
}
SafetyStatus MarineSafety::unlock(const std::string& operatorName, uint64_t nowMs) { unlockExpiresAt_ = nowMs + 600000; audit("safety_unlock", operatorName + " 请求本地控制解锁。", nowMs); return status(nowMs); }
SafetyStatus MarineSafety::lock(const std::string& operatorName, uint64_t nowMs) { unlockExpiresAt_ = 0; audit("safety_lock", operatorName + " 锁定执行器控制。", nowMs); return status(nowMs); }
bool MarineSafety::mayControl(const Binding& binding, uint64_t nowMs) {
    const bool allowed = status(nowMs).actuatorControlEnabled && binding.realControlEnabled;
    audit(allowed ? "control_allowed" : "control_rejected", binding.logicalExecutorId + (allowed ? " 控制许可。" : " 控制被安全闸门拒绝。"), nowMs);
    return allowed;
}
void MarineSafety::emergencyStop(const std::string& operatorName, uint64_t nowMs) { emergencyStopped_ = true; audit("emergency_stop", operatorName + " 触发执行器急停。", nowMs); }
void MarineSafety::resetEmergency(const std::string& operatorName, uint64_t nowMs) { emergencyStopped_ = false; audit("emergency_reset", operatorName + " 复位执行器急停。", nowMs); }
void MarineSafety::audit(const std::string& type, const std::string& message, uint64_t nowMs) const { std::ofstream stream(eventPath_, std::ios::app); if (stream) stream << toJson(JsonValue::Object{{"type", type}, {"message", message}, {"at", static_cast<double>(nowMs)}}) << '\n'; }
} // namespace marine
