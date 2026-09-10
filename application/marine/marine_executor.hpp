#ifndef MARINE_EXECUTOR_HPP
#define MARINE_EXECUTOR_HPP

#include "marine_types.hpp"

#include <functional>
#include <map>
#include <string>

namespace marine {

struct SensorSnapshot {
    std::string source;
    std::map<std::string, double> values;
    std::map<std::string, bool> online;
    static SensorSnapshot demo();
};
struct ServiceResult { std::string metric; std::string value; std::string unit; std::string detail; std::string source; };

class MarineExecutor {
public:
    using SensorReader = std::function<SensorSnapshot()>;
    explicit MarineExecutor(SensorReader sensorReader);
    SensorSnapshot snapshot() const;
    ServiceResult execute(const TaskNode& node, double progress, const SensorSnapshot& sensors) const;

private:
    SensorReader sensorReader_;
};

} // namespace marine

#endif
