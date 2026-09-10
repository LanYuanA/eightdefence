#ifndef MARINE_TYPES_HPP
#define MARINE_TYPES_HPP

#include "json_value.hpp"

#include <string>
#include <vector>

namespace marine {

struct TaskNode { std::string id; std::string serviceId; std::string area; int intensity = 70; int duration = 6; };
struct TaskStep { std::string id; std::vector<TaskNode> nodes; };
struct Application { std::string id; std::string name; std::string description; std::vector<TaskStep> steps; };

enum class RunStatus { Idle, Running, Paused, Completed, Cancelled, Failed, Interrupted };
enum class NodeStatus { Waiting, Running, Completed, Cancelled, Failed };

struct Binding {
    std::string logicalExecutorId;
    std::string deviceId;
    std::string bus;
    std::string address;
    std::vector<std::string> capabilities;
    bool realControlEnabled = false;
    uint64_t version = 1;
    uint64_t updatedAt = 0;
};

struct RunSummary {
    std::string id;
    std::string appId;
    RunStatus status = RunStatus::Idle;
    int stepIndex = 0;
};

bool isAvailableService(const std::string& serviceId);
bool isAwarenessService(const std::string& serviceId);
std::string serviceResource(const std::string& serviceId);
std::vector<std::string> validateApplication(const Application& app);
JsonValue applicationToJson(const Application& app);
bool applicationFromJson(const JsonValue& value, Application& app, std::string& error);
std::string runStatusToString(RunStatus status);
bool runStatusFromString(const std::string& value, RunStatus& status);

} // namespace marine

#endif
