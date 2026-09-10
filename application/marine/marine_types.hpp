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

bool isAvailableService(const std::string& serviceId);
bool isAwarenessService(const std::string& serviceId);
std::string serviceResource(const std::string& serviceId);
std::vector<std::string> validateApplication(const Application& app);
JsonValue applicationToJson(const Application& app);
bool applicationFromJson(const JsonValue& value, Application& app, std::string& error);

} // namespace marine

#endif
