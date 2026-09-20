#ifndef SVC_MOTOR_SPEED_HPP
#define SVC_MOTOR_SPEED_HPP

#include "application/marine/marine_types.hpp"

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace marine {

struct MotorTelemetry {
    std::string executorId;
    std::string name;
    std::string model;
    std::string address;
    std::string owner;
    bool online = true;
    bool running = false;
    int targetRpm = 0;
    int actualRpm = 0;
    std::string direction = "forward";
    uint16_t statusWord = 0;
    std::string source = "simulation";
};

class MotorAtomicService {
public:
    using Writer = std::function<bool(uint8_t, uint16_t, const std::vector<uint16_t>&)>;
    using Reader = std::function<bool(uint8_t, uint16_t, uint16_t, std::vector<uint16_t>&)>;
    using EmergencyWriter = std::function<std::vector<bool>(const std::vector<uint8_t>&)>;
    explicit MotorAtomicService(bool simulation = true);
    void attachIo(Writer writer, Reader reader, EmergencyWriter emergencyWriter = {});
    std::string start(const std::string& owner, const MotorParameters& parameters);
    std::string update(const std::string& owner, const MotorParameters& parameters);
    std::string stop(const std::string& owner, const std::string& executorId);
    MotorTelemetry telemetry(const std::string& executorId);
    bool emergencyStop();
    void resetEmergency();
    bool emergencyStopped() const;
    bool simulation() const;
    std::vector<MotorTelemetry> list();
    std::vector<MotorTelemetry> scanAll();

private:
    struct Device { MotorTelemetry telemetry; uint8_t address; bool directionInverted = false; uint64_t lastReadMs = 0; uint64_t generation = 0; };
    bool writeOne(uint8_t address, uint16_t reg, uint16_t value, const Writer& writer, bool simulation) const;
    bool writeI32(uint8_t address, uint16_t reg, int32_t value, const Writer& writer, bool simulation) const;
    bool operationAllowed(const std::string& executorId, const std::string& owner, uint64_t generation) const;
    std::string signedDirection(const MotorParameters& parameters, const Device& device, int& signedRpm) const;
    void refreshInventory();
    mutable std::mutex mutex_;
    std::map<std::string, Device> devices_;
    Writer writer_;
    Reader reader_;
    EmergencyWriter emergencyWriter_;
    bool simulation_;
    bool emergencyStopped_ = false;
    size_t discoveryIndex_ = 0;
    std::string detectedExecutorId_;
};

} // namespace marine
#endif
