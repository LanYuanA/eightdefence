#include "marine_types.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace marine {
namespace {
const std::map<std::string, std::string> kServices = {
    {"A01", ""}, {"A02", ""}, {"A03", ""}, {"A04", ""}, {"A05", ""}, {"A06", ""}, {"A07", ""},
    {"01", "通风执行器"}, {"02", "冷却执行器"}, {"03", "水务执行器"}, {"04", "水务执行器"}, {"05", "安全报警执行器"}
};
const std::set<std::string> kAwareness = {"A01", "A02", "A03", "A04", "A05", "A06", "A07"};
const std::set<std::string> kAreas = {"机舱", "生活舱", "作业舱", "全船"};

bool readString(const JsonValue::Object& object, const std::string& key, std::string& output, std::string& error) {
    const auto iterator = object.find(key);
    if (iterator == object.end() || !iterator->second.isString()) { error = "缺少字符串字段: " + key; return false; }
    output = iterator->second.asString(); return true;
}
bool readInteger(const JsonValue::Object& object, const std::string& key, int& output, std::string& error) {
    const auto iterator = object.find(key);
    if (iterator == object.end() || !iterator->second.isNumber() || std::floor(iterator->second.asNumber()) != iterator->second.asNumber()) { error = "缺少整数字段: " + key; return false; }
    output = static_cast<int>(iterator->second.asNumber()); return true;
}
int defaultThreshold(const std::string& serviceId) {
    if (serviceId == "A01") return 30;
    if (serviceId == "A02") return 35;
    if (serviceId == "A03") return 800;
    if (serviceId == "A07") return 300;
    return 1;
}
}

bool isAvailableService(const std::string& serviceId) { return kServices.find(serviceId) != kServices.end(); }
bool isAwarenessService(const std::string& serviceId) { return kAwareness.find(serviceId) != kAwareness.end(); }
std::string serviceResource(const std::string& serviceId) { const auto iterator = kServices.find(serviceId); return iterator == kServices.end() ? "" : iterator->second; }

std::vector<std::string> validateApplication(const Application& app) {
    std::vector<std::string> errors;
    if (app.name.empty()) errors.push_back("请填写应用名称。");
    if (app.steps.empty()) errors.push_back("请先添加至少一个步骤。");
    std::set<std::string> nodeIds;
    for (size_t stepIndex = 0; stepIndex < app.steps.size(); ++stepIndex) {
        const auto& step = app.steps[stepIndex];
        if (step.nodes.empty()) { errors.push_back("步骤 " + std::to_string(stepIndex + 1) + " 还没有服务。"); continue; }
        std::set<std::string> resources;
        for (const auto& node : step.nodes) {
            if (!isAvailableService(node.serviceId)) { errors.push_back("步骤 " + std::to_string(stepIndex + 1) + " 包含尚未实现的服务。"); continue; }
            if (!nodeIds.insert(node.id).second) errors.push_back("存在重复的服务实例。");
            if (kAreas.find(node.area) == kAreas.end() || node.intensity < 10 || node.intensity > 100 || node.duration < 2 || node.duration > 30 || (isAwarenessService(node.serviceId) && (node.threshold < 0 || (node.thresholdOperator != "gte" && node.thresholdOperator != "lte")))) errors.push_back(node.serviceId + " 的参数不完整或超出范围。");
            const std::string resource = serviceResource(node.serviceId);
            if (!resource.empty() && !resources.insert(resource).second) errors.push_back("步骤 " + std::to_string(stepIndex + 1) + " 的服务共用" + resource + "，请拆成顺序步骤。");
        }
    }
    return errors;
}

JsonValue applicationToJson(const Application& app) {
    JsonValue::Object object{{"id", app.id}, {"name", app.name}, {"description", app.description}};
    JsonValue::Array steps;
    for (const auto& step : app.steps) {
        JsonValue::Array nodes;
        for (const auto& node : step.nodes) nodes.emplace_back(JsonValue::Object{{"id", node.id}, {"serviceId", node.serviceId}, {"area", node.area}, {"intensity", node.intensity}, {"duration", node.duration}, {"threshold", node.threshold}, {"thresholdOperator", node.thresholdOperator}});
        steps.emplace_back(JsonValue::Object{{"id", step.id}, {"nodes", std::move(nodes)}});
    }
    object.emplace("steps", std::move(steps));
    return JsonValue(std::move(object));
}

bool applicationFromJson(const JsonValue& value, Application& app, std::string& error) {
    if (!value.isObject()) { error = "应用定义必须是对象。"; return false; }
    const auto& object = value.asObject();
    Application parsed;
    if (!readString(object, "id", parsed.id, error) || !readString(object, "name", parsed.name, error) || !readString(object, "description", parsed.description, error)) return false;
    const auto steps = value.get("steps");
    if (steps == nullptr || !steps->isArray()) { error = "缺少步骤数组。"; return false; }
    for (const auto& rawStep : steps->asArray()) {
        if (!rawStep.isObject()) { error = "步骤必须是对象。"; return false; }
        TaskStep step;
        const auto& stepObject = rawStep.asObject();
        if (!readString(stepObject, "id", step.id, error)) return false;
        const auto nodeArray = rawStep.get("nodes");
        if (nodeArray == nullptr || !nodeArray->isArray()) { error = "步骤缺少服务数组。"; return false; }
        for (const auto& rawNode : nodeArray->asArray()) {
            if (!rawNode.isObject()) { error = "服务节点必须是对象。"; return false; }
            TaskNode node;
            const auto& nodeObject = rawNode.asObject();
            if (!readString(nodeObject, "id", node.id, error) || !readString(nodeObject, "serviceId", node.serviceId, error) || !readString(nodeObject, "area", node.area, error) || !readInteger(nodeObject, "intensity", node.intensity, error) || !readInteger(nodeObject, "duration", node.duration, error)) return false;
            node.threshold = defaultThreshold(node.serviceId);
            const auto threshold = nodeObject.find("threshold");
            if (threshold != nodeObject.end()) {
                if (!threshold->second.isNumber() || std::floor(threshold->second.asNumber()) != threshold->second.asNumber()) { error = "阈值必须是整数。"; return false; }
                node.threshold = static_cast<int>(threshold->second.asNumber());
            }
            const auto thresholdOperator = nodeObject.find("thresholdOperator");
            if (thresholdOperator != nodeObject.end()) {
                if (!thresholdOperator->second.isString()) { error = "阈值关系必须是字符串。"; return false; }
                node.thresholdOperator = thresholdOperator->second.asString();
            } else if (node.serviceId == "A07") node.thresholdOperator = "lte";
            step.nodes.push_back(std::move(node));
        }
        parsed.steps.push_back(std::move(step));
    }
    const auto errors = validateApplication(parsed);
    if (!errors.empty()) { error = errors.front(); return false; }
    app = std::move(parsed);
    return true;
}

} // namespace marine
