#include "svc_motor_speed.hpp"

#include <array>
#include <chrono>
#include <thread>
#include <cmath>

namespace marine {
namespace {
std::vector<uint16_t> i32Words(int32_t value) { const uint32_t raw = static_cast<uint32_t>(value); return {static_cast<uint16_t>(raw >> 16), static_cast<uint16_t>(raw)}; }
uint64_t steadyMs() { return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()); }
}

MotorAtomicService::MotorAtomicService(bool simulation) : simulation_(simulation) {
    devices_.emplace("EXHAUST-FAN-01", Device{{"EXHAUST-FAN-01", "机舱排烟风机", "IDS57-R", "0x02"}, 0x02});
    devices_.emplace("FIRE-PUMP-01", Device{{"FIRE-PUMP-01", "消防水泵", "IDS42-R", "0x0E"}, 0x0E});
    devices_.emplace("DRAIN-PUMP-01", Device{{"DRAIN-PUMP-01", "舱底排水泵", "IDS42-R", "0x0F"}, 0x0F});
}
void MotorAtomicService::attachIo(Writer writer, Reader reader) { std::lock_guard<std::mutex> lock(mutex_); writer_ = std::move(writer); reader_ = std::move(reader); simulation_ = false; for (auto& item : devices_) item.second.telemetry.source = "device"; }
bool MotorAtomicService::writeOne(Device& device, uint16_t reg, uint16_t value) { return simulation_ || (writer_ && writer_(device.address, reg, {value})); }
bool MotorAtomicService::writeI32(Device& device, uint16_t reg, int32_t value) { return simulation_ || (writer_ && writer_(device.address, reg, i32Words(value))); }
std::string MotorAtomicService::signedDirection(const MotorParameters& p, const Device& d, int& rpm) const { const bool reverse = (p.direction == MotorDirection::Reverse) != d.directionInverted; rpm = reverse ? -p.speedRpm : p.speedRpm; return reverse ? "reverse" : "forward"; }
std::string MotorAtomicService::start(const std::string& owner, const MotorParameters& p) {
    std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(p.executorId); if (found == devices_.end()) return "unknown_executor"; if (p.speedRpm < 1 || p.speedRpm > 500 || p.acceleration < 1 || p.acceleration > 100 || p.deceleration < 1 || p.deceleration > 100) return "invalid_parameters"; if (emergencyStopped_) return "emergency_stopped"; auto& d = found->second; if (!d.telemetry.owner.empty() && d.telemetry.owner != owner) return "resource_busy"; int rpm = 0; const auto direction = signedDirection(p, d, rpm);
    d.telemetry.owner = owner;
    if (!writeOne(d, 0x6040, 6) || !writeOne(d, 0x6040, 7) || !writeOne(d, 0x6040, 15) || !writeOne(d, 0x6060, 3) || !writeI32(d, 0x6083, p.acceleration) || !writeI32(d, 0x6084, p.deceleration) || !writeI32(d, 0x60FF, rpm)) { writeOne(d, 0x6040, 6); d.telemetry.owner.clear(); d.telemetry.running = false; d.telemetry.online = false; return "device_error"; }
    d.telemetry.online = true; d.telemetry.running = true; d.telemetry.targetRpm = rpm; d.telemetry.actualRpm = simulation_ ? rpm : 0; d.telemetry.direction = direction; d.telemetry.statusWord = 0x027; return "";
}
std::string MotorAtomicService::update(const std::string& owner, const MotorParameters& p) { std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(p.executorId); if (found == devices_.end()) return "unknown_executor"; if (p.speedRpm < 1 || p.speedRpm > 500 || p.acceleration < 1 || p.acceleration > 100 || p.deceleration < 1 || p.deceleration > 100) return "invalid_parameters"; auto& d = found->second; if (d.telemetry.owner != owner) return "resource_busy"; if (emergencyStopped_) return "emergency_stopped"; int rpm = 0; const auto direction = signedDirection(p, d, rpm); if (direction != d.telemetry.direction) { if (!writeI32(d, 0x60FF, 0)) return "device_error"; if (!simulation_) { bool stopped = false; for (int attempt = 0; attempt < 20; ++attempt) { std::vector<uint16_t> words; if (reader_ && reader_(d.address, 0x606C, 1, words) && !words.empty() && std::abs(static_cast<int16_t>(words[0])) <= 5) { stopped = true; break; } std::this_thread::sleep_for(std::chrono::milliseconds(100)); } if (!stopped) { writeOne(d, 0x6040, 6); d.telemetry.running = false; d.telemetry.owner.clear(); return "direction_change_timeout"; } } }
    if (!writeI32(d, 0x6083, p.acceleration) || !writeI32(d, 0x6084, p.deceleration) || !writeI32(d, 0x60FF, rpm)) return "device_error";
    d.telemetry.targetRpm = rpm;
    d.telemetry.actualRpm = simulation_ ? rpm : d.telemetry.actualRpm;
    d.telemetry.direction = direction;
    d.telemetry.running = true;
    return "";
}
std::string MotorAtomicService::stop(const std::string& owner, const std::string& id) { std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(id); if (found == devices_.end()) return "unknown_executor"; auto& d = found->second; if (!owner.empty() && !d.telemetry.owner.empty() && d.telemetry.owner != owner) return "resource_busy"; const bool ok = writeOne(d, 0x6040, 6); d.telemetry.owner.clear(); d.telemetry.running = false; d.telemetry.targetRpm = 0; d.telemetry.actualRpm = 0; return ok ? "" : "device_error"; }
MotorTelemetry MotorAtomicService::telemetry(const std::string& id) { std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(id); if (found == devices_.end()) return {}; auto& d = found->second; const auto now = steadyMs(); if (!simulation_ && reader_ && now - d.lastReadMs >= 500) { d.lastReadMs = now; std::vector<uint16_t> words; if (reader_(d.address, 0x606C, 1, words) && !words.empty()) d.telemetry.actualRpm = static_cast<int16_t>(words[0]); else d.telemetry.online = false; if (reader_(d.address, 0x6041, 1, words) && !words.empty()) { d.telemetry.statusWord = words[0]; d.telemetry.running = std::abs(d.telemetry.actualRpm) > 5 || (!d.telemetry.owner.empty() && d.telemetry.targetRpm != 0); d.telemetry.online = true; } }
    return d.telemetry; }
void MotorAtomicService::emergencyStop() { std::lock_guard<std::mutex> lock(mutex_); emergencyStopped_ = true; for (auto& item : devices_) { writeOne(item.second, 0x6040, 6); item.second.telemetry.owner.clear(); item.second.telemetry.running = false; item.second.telemetry.targetRpm = 0; item.second.telemetry.actualRpm = 0; } }
void MotorAtomicService::resetEmergency() { std::lock_guard<std::mutex> lock(mutex_); emergencyStopped_ = false; }
bool MotorAtomicService::emergencyStopped() const { std::lock_guard<std::mutex> lock(mutex_); return emergencyStopped_; }
bool MotorAtomicService::simulation() const { std::lock_guard<std::mutex> lock(mutex_); return simulation_; }
std::vector<MotorTelemetry> MotorAtomicService::list() { std::vector<MotorTelemetry> result; for (const auto& id : {"EXHAUST-FAN-01", "FIRE-PUMP-01", "DRAIN-PUMP-01"}) result.push_back(telemetry(id)); return result; }
} // namespace marine
