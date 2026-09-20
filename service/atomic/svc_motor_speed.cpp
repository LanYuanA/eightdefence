#include "svc_motor_speed.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <thread>

namespace marine {
namespace {
std::vector<uint16_t> i32Words(int32_t value) { const uint32_t raw = static_cast<uint32_t>(value); return {static_cast<uint16_t>(raw >> 16), static_cast<uint16_t>(raw)}; }
int32_t wordsI32(const std::vector<uint16_t>& words) { return static_cast<int32_t>((static_cast<uint32_t>(words[0]) << 16) | words[1]); }
uint64_t steadyMs() { return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()); }
bool valid(const MotorParameters& p) { return p.speedRpm >= 1 && p.speedRpm <= 500 && p.acceleration >= 1 && p.acceleration <= 100 && p.deceleration >= 1 && p.deceleration <= 100; }
}

MotorAtomicService::MotorAtomicService(bool simulation) : simulation_(simulation) {
    devices_.emplace("EXHAUST-FAN-01", Device{{"EXHAUST-FAN-01", "机舱排烟风机", "IDS57-R", "0x02"}, 0x02});
    devices_.emplace("FIRE-PUMP-01", Device{{"FIRE-PUMP-01", "消防水泵", "IDS42-R", "0x0E"}, 0x0E});
    devices_.emplace("DRAIN-PUMP-01", Device{{"DRAIN-PUMP-01", "舱底排水泵", "IDS42-R", "0x0F"}, 0x0F});
}

void MotorAtomicService::attachIo(Writer writer, Reader reader, EmergencyWriter emergencyWriter) {
    std::lock_guard<std::mutex> lock(mutex_); writer_ = std::move(writer); reader_ = std::move(reader); emergencyWriter_ = std::move(emergencyWriter); simulation_ = false;
    for (auto& item : devices_) { item.second.telemetry.source = "device"; item.second.telemetry.online = false; }
}
bool MotorAtomicService::writeOne(uint8_t address, uint16_t reg, uint16_t value, const Writer& writer, bool simulation) const { return simulation || (writer && writer(address, reg, {value})); }
bool MotorAtomicService::writeI32(uint8_t address, uint16_t reg, int32_t value, const Writer& writer, bool simulation) const { return simulation || (writer && writer(address, reg, i32Words(value))); }
bool MotorAtomicService::operationAllowed(const std::string& id, const std::string& owner, uint64_t generation) const {
    std::lock_guard<std::mutex> lock(mutex_); const auto found = devices_.find(id);
    return found != devices_.end() && !emergencyStopped_ && found->second.generation == generation && found->second.telemetry.owner == owner;
}
std::string MotorAtomicService::signedDirection(const MotorParameters& p, const Device& d, int& rpm) const { const bool reverse = (p.direction == MotorDirection::Reverse) != d.directionInverted; rpm = reverse ? -p.speedRpm : p.speedRpm; return reverse ? "reverse" : "forward"; }

std::string MotorAtomicService::start(const std::string& owner, const MotorParameters& p) {
    uint8_t address; uint64_t generation; int rpm = 0; std::string direction; Writer writer; bool simulation;
    {
        std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(p.executorId);
        if (found == devices_.end()) return "unknown_executor";
        if (!valid(p)) return "invalid_parameters";
        if (emergencyStopped_) return "emergency_stopped";
        auto& d = found->second;
        if (!d.telemetry.owner.empty() && d.telemetry.owner != owner) return "resource_busy";
        direction = signedDirection(p, d, rpm); d.telemetry.owner = owner; generation = ++d.generation; address = d.address; writer = writer_; simulation = simulation_;
    }
    bool ok = true;
    for (const auto& command : std::array<std::pair<uint16_t, uint16_t>, 4>{{{0x6040, 6}, {0x6040, 7}, {0x6040, 15}, {0x6060, 3}}}) {
        if (!operationAllowed(p.executorId, owner, generation) || !writeOne(address, command.first, command.second, writer, simulation)) { ok = false; break; }
    }
    for (const auto& command : std::array<std::pair<uint16_t, int32_t>, 3>{{{0x6083, p.acceleration}, {0x6084, p.deceleration}, {0x60FF, rpm}}}) {
        if (!ok || !operationAllowed(p.executorId, owner, generation) || !writeI32(address, command.first, command.second, writer, simulation)) { ok = false; break; }
    }
    if (!ok) {
        writeOne(address, 0x6040, 6, writer, simulation); std::lock_guard<std::mutex> lock(mutex_); auto& d = devices_.at(p.executorId);
        if (d.generation == generation) { d.telemetry.owner.clear(); d.telemetry.running = false; d.telemetry.online = false; }
        return emergencyStopped_ ? "emergency_stopped" : "device_error";
    }
    std::lock_guard<std::mutex> lock(mutex_); auto& d = devices_.at(p.executorId);
    if (d.generation != generation || emergencyStopped_) return "emergency_stopped";
    d.telemetry.online = true; d.telemetry.running = true; d.telemetry.targetRpm = rpm; d.telemetry.actualRpm = simulation ? rpm : 0; d.telemetry.direction = direction; d.telemetry.statusWord = 0x027; return "";
}

std::string MotorAtomicService::update(const std::string& owner, const MotorParameters& p) {
    uint8_t address; uint64_t generation; int rpm = 0; std::string oldDirection, direction; Writer writer; Reader reader; bool simulation;
    {
        std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(p.executorId);
        if (found == devices_.end()) return "unknown_executor";
        if (!valid(p)) return "invalid_parameters";
        auto& d = found->second;
        if (d.telemetry.owner != owner) return "resource_busy";
        if (emergencyStopped_) return "emergency_stopped";
        direction = signedDirection(p, d, rpm); oldDirection = d.telemetry.direction; generation = d.generation; address = d.address; writer = writer_; reader = reader_; simulation = simulation_;
    }
    if (direction != oldDirection) {
        if (!writeI32(address, 0x60FF, 0, writer, simulation)) return "device_error";
        if (!simulation) {
            bool stopped = false;
            for (int attempt = 0; attempt < 20 && operationAllowed(p.executorId, owner, generation); ++attempt) {
                std::vector<uint16_t> words;
                if (reader && reader(address, 0x606C, 2, words) && words.size() == 2 && std::abs(wordsI32(words)) <= 5) { stopped = true; break; }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (!stopped) {
                writeOne(address, 0x6040, 6, writer, simulation); std::lock_guard<std::mutex> lock(mutex_); auto& d = devices_.at(p.executorId);
                if (d.generation == generation) { d.telemetry.running = false; d.telemetry.owner.clear(); ++d.generation; }
                return emergencyStopped_ ? "emergency_stopped" : "direction_change_timeout";
            }
        }
    }
    if (!operationAllowed(p.executorId, owner, generation)) return "emergency_stopped";
    if (!writeI32(address, 0x6083, p.acceleration, writer, simulation) || !writeI32(address, 0x6084, p.deceleration, writer, simulation) || !writeI32(address, 0x60FF, rpm, writer, simulation)) return "device_error";
    std::lock_guard<std::mutex> lock(mutex_); auto& d = devices_.at(p.executorId);
    if (d.generation != generation || emergencyStopped_) return "emergency_stopped";
    d.telemetry.targetRpm = rpm; if (simulation) d.telemetry.actualRpm = rpm; d.telemetry.direction = direction; d.telemetry.running = true; return "";
}

std::string MotorAtomicService::stop(const std::string& owner, const std::string& id) {
    uint8_t address; Writer writer; bool simulation;
    {
        std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(id); if (found == devices_.end()) return "unknown_executor";
        if (!owner.empty() && !found->second.telemetry.owner.empty() && found->second.telemetry.owner != owner) return "resource_busy";
        address = found->second.address; writer = writer_; simulation = simulation_; ++found->second.generation;
    }
    const bool ok = writeOne(address, 0x6040, 6, writer, simulation); std::lock_guard<std::mutex> lock(mutex_); auto& d = devices_.at(id);
    if (ok) { d.telemetry.owner.clear(); d.telemetry.running = false; d.telemetry.targetRpm = 0; d.telemetry.actualRpm = 0; } else d.telemetry.online = false;
    return ok ? "" : "device_error";
}

MotorTelemetry MotorAtomicService::telemetry(const std::string& id) {
    uint8_t address; uint64_t generation; Reader reader; bool readSpeed = false;
    {
        std::lock_guard<std::mutex> lock(mutex_); auto found = devices_.find(id); if (found == devices_.end()) return {}; auto& d = found->second; const auto now = steadyMs();
        if (simulation_ || !reader_ || now - d.lastReadMs < 500) return d.telemetry;
        d.lastReadMs = now; address = d.address; generation = d.generation; reader = reader_; readSpeed = d.telemetry.running || !d.telemetry.owner.empty() || d.telemetry.targetRpm != 0;
    }
    std::vector<uint16_t> speed, status; const bool statusOk = reader(address, 0x6041, 1, status) && status.size() == 1;
    const bool speedOk = readSpeed && statusOk && reader(address, 0x606C, 2, speed) && speed.size() == 2;
    std::lock_guard<std::mutex> lock(mutex_); auto& d = devices_.at(id); if (d.generation != generation) return d.telemetry;
    if (speedOk) d.telemetry.actualRpm = wordsI32(speed);
    if (statusOk) d.telemetry.statusWord = status[0];
    // 状态字是设备在线的心跳依据；转速寄存器偶发超时不应把整台电机判为离线。
    d.telemetry.online = statusOk;
    d.telemetry.running = d.telemetry.online && (std::abs(d.telemetry.actualRpm) > 5 || (!d.telemetry.owner.empty() && d.telemetry.targetRpm != 0)); return d.telemetry;
}

bool MotorAtomicService::emergencyStop() {
    std::vector<std::pair<std::string, uint8_t>> targets; Writer writer; EmergencyWriter emergencyWriter; bool simulation;
    {
        std::lock_guard<std::mutex> lock(mutex_); emergencyStopped_ = true; writer = writer_; emergencyWriter = emergencyWriter_; simulation = simulation_;
        for (auto& item : devices_) { ++item.second.generation; targets.emplace_back(item.first, item.second.address); }
    }
    std::vector<bool> outcomes;
    if (simulation) outcomes.assign(targets.size(), true);
    else if (emergencyWriter) { std::vector<uint8_t> addresses; for (const auto& target : targets) addresses.push_back(target.second); outcomes = emergencyWriter(addresses); }
    else for (const auto& target : targets) outcomes.push_back(writeOne(target.second, 0x6040, 6, writer, false));
    bool allStopped = outcomes.size() == targets.size();
    for (size_t index = 0; index < targets.size(); ++index) {
        const auto& target = targets[index]; const bool stopped = index < outcomes.size() && outcomes[index]; allStopped = allStopped && stopped; std::lock_guard<std::mutex> lock(mutex_); auto& d = devices_.at(target.first);
        if (stopped) { d.telemetry.owner.clear(); d.telemetry.running = false; d.telemetry.targetRpm = 0; d.telemetry.actualRpm = 0; } else d.telemetry.online = false;
    }
    return allStopped;
}
void MotorAtomicService::resetEmergency() { std::lock_guard<std::mutex> lock(mutex_); emergencyStopped_ = false; }
bool MotorAtomicService::emergencyStopped() const { std::lock_guard<std::mutex> lock(mutex_); return emergencyStopped_; }
bool MotorAtomicService::simulation() const { std::lock_guard<std::mutex> lock(mutex_); return simulation_; }
std::vector<MotorTelemetry> MotorAtomicService::list() {
    refreshInventory();
    static const std::array<const char*, 3> ids{{"EXHAUST-FAN-01", "FIRE-PUMP-01", "DRAIN-PUMP-01"}};
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MotorTelemetry> result; result.reserve(ids.size());
    for (const auto* id : ids) result.push_back(devices_.at(id).telemetry);
    return result;
}

void MotorAtomicService::refreshInventory() {
    static const std::array<const char*, 3> ids{{"EXHAUST-FAN-01", "FIRE-PUMP-01", "DRAIN-PUMP-01"}};
    std::string id; uint8_t address; uint64_t generation; Reader reader;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (simulation_ || !reader_) return;
        if (!detectedExecutorId_.empty()) id = detectedExecutorId_;
        else { id = ids[discoveryIndex_]; discoveryIndex_ = (discoveryIndex_ + 1) % ids.size(); }
        const auto& device = devices_.at(id);
        address = device.address; generation = device.generation; reader = reader_;
    }

    std::vector<uint16_t> status, speed;
    const bool online = reader(address, 0x6041, 1, status) && status.size() == 1;
    // Refresh measured speed on the same detected address, including after restart.
    // A speed-read failure alone must not mark a responding motor offline.
    const bool speedOk = online && reader(address, 0x606C, 2, speed) && speed.size() == 2;
    std::lock_guard<std::mutex> lock(mutex_);
    auto& device = devices_.at(id);
    if (device.generation != generation) return;
    if (online) {
        detectedExecutorId_ = id;
        device.telemetry.statusWord = status[0];
        if (speedOk) {
            device.telemetry.actualRpm = wordsI32(speed);
            device.telemetry.running = std::abs(device.telemetry.actualRpm) > 5;
            if (device.telemetry.actualRpm != 0)
                device.telemetry.direction = device.telemetry.actualRpm < 0 ? "reverse" : "forward";
        }
        for (auto& item : devices_) item.second.telemetry.online = item.first == id;
        return;
    }
    device.telemetry.online = false;
    if (detectedExecutorId_ == id) {
        detectedExecutorId_.clear();
        for (auto& item : devices_) item.second.telemetry.online = false;
    }
}
} // namespace marine
