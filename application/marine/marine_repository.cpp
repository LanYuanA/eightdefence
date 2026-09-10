#include "marine_repository.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace marine {
namespace {
namespace fs = std::filesystem;

uint64_t nowMs() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}
std::string readFile(const fs::path& path, std::string& output) {
    std::ifstream stream(path);
    if (!stream) return "无法读取 " + path.string();
    std::ostringstream buffer; buffer << stream.rdbuf(); output = buffer.str(); return "";
}
std::string writeAtomically(const fs::path& path, const std::string& content) {
    const fs::path temporary = path.string() + ".tmp";
    { std::ofstream stream(temporary, std::ios::trunc); if (!stream) return "无法写入 " + temporary.string(); stream << content; if (!stream.good()) return "写入 " + temporary.string() + " 失败。"; }
    std::error_code error; fs::rename(temporary, path, error);
    if (error) { fs::remove(path, error); error.clear(); fs::rename(temporary, path, error); }
    return error ? "无法替换 " + path.string() : "";
}
const JsonValue* field(const JsonValue::Object& object, const std::string& name) { const auto iterator = object.find(name); return iterator == object.end() ? nullptr : &iterator->second; }
bool stringField(const JsonValue::Object& object, const std::string& name, std::string& output) { const auto value = field(object, name); if (value == nullptr || !value->isString()) return false; output = value->asString(); return true; }
bool numberField(const JsonValue::Object& object, const std::string& name, uint64_t& output) { const auto value = field(object, name); if (value == nullptr || !value->isNumber() || value->asNumber() < 0) return false; output = static_cast<uint64_t>(value->asNumber()); return true; }
JsonValue bindingToJson(const Binding& binding) { JsonValue::Array capabilities; for (const auto& capability : binding.capabilities) capabilities.emplace_back(capability); return JsonValue::Object{{"logicalExecutorId", binding.logicalExecutorId}, {"deviceId", binding.deviceId}, {"bus", binding.bus}, {"address", binding.address}, {"capabilities", std::move(capabilities)}, {"realControlEnabled", binding.realControlEnabled}, {"version", static_cast<double>(binding.version)}, {"updatedAt", static_cast<double>(binding.updatedAt)}}; }
bool bindingFromJson(const JsonValue& value, Binding& binding) {
    if (!value.isObject()) return false;
    const auto& object = value.asObject();
    if (!stringField(object, "logicalExecutorId", binding.logicalExecutorId) || !stringField(object, "deviceId", binding.deviceId) || !stringField(object, "bus", binding.bus) || !stringField(object, "address", binding.address) || !numberField(object, "version", binding.version) || !numberField(object, "updatedAt", binding.updatedAt)) return false;
    const auto capabilities = field(object, "capabilities"); if (capabilities == nullptr || !capabilities->isArray()) return false;
    binding.capabilities.clear(); for (const auto& item : capabilities->asArray()) { if (!item.isString()) return false; binding.capabilities.push_back(item.asString()); }
    const auto real = field(object, "realControlEnabled"); binding.realControlEnabled = real != nullptr && real->isBool() && real->asBool(); return true;
}
JsonValue runToJson(const RunSummary& run) { return JsonValue::Object{{"id", run.id}, {"appId", run.appId}, {"status", runStatusToString(run.status)}, {"stepIndex", run.stepIndex}}; }
bool runFromJson(const JsonValue& value, RunSummary& run) { if (!value.isObject()) return false; const auto& object = value.asObject(); std::string status; uint64_t step = 0; if (!stringField(object, "id", run.id) || !stringField(object, "appId", run.appId) || !stringField(object, "status", status) || !numberField(object, "stepIndex", step) || !runStatusFromString(status, run.status)) return false; run.stepIndex = static_cast<int>(step); return true; }
std::vector<Binding> defaultBindings() {
    const uint64_t timestamp = nowMs();
    return {{"VENT-01", "MOTOR-02", "RS485", "0x02", {"ventilation", "speed", "status"}, false, 1, timestamp}, {"COOL-01", "MOTOR-0E", "RS485", "0x0E", {"cooling", "status"}, false, 1, timestamp}, {"WATER-01", "MOTOR-0E", "RS485", "0x0E", {"water", "status"}, false, 1, timestamp}, {"DRAIN-01", "MOTOR-0F", "RS485", "0x0F", {"drainage", "status"}, false, 1, timestamp}};
}
}

MarineRepository::MarineRepository(std::string root) : root_(std::move(root)) {}
std::string MarineRepository::load() {
    std::error_code error; fs::create_directories(root_, error); if (error) return "无法创建数据目录。";
    if (const auto result = readApps(); !result.empty()) return result;
    if (const auto result = readBindings(); !result.empty()) return result;
    if (const auto result = readRuns(); !result.empty()) return result;
    bool changed = false;
    for (auto& run : runs_) if (run.status == RunStatus::Running || run.status == RunStatus::Paused) { run.status = RunStatus::Interrupted; changed = true; }
    if (changed) { if (const auto result = writeRuns(); !result.empty()) return result; appendEvent("gateway_restart", "网关重启，未完成任务已中断。"); }
    return "";
}
const std::vector<Application>& MarineRepository::listApps() const { return apps_; }
std::string MarineRepository::saveApp(const Application& app) {
    const auto errors = validateApplication(app); if (!errors.empty()) return errors.front();
    const auto iterator = std::find_if(apps_.begin(), apps_.end(), [&](const Application& current) { return current.id == app.id; });
    if (iterator == apps_.end()) apps_.push_back(app); else *iterator = app;
    return writeApps();
}
std::string MarineRepository::removeApp(const std::string& appId) { const auto original = apps_.size(); apps_.erase(std::remove_if(apps_.begin(), apps_.end(), [&](const Application& app) { return app.id == appId; }), apps_.end()); return apps_.size() == original ? "not_found" : writeApps(); }
const std::vector<Binding>& MarineRepository::listBindings() const { return bindings_; }
std::string MarineRepository::updateBinding(const Binding& binding, uint64_t expectedVersion, const std::string& operatorName) {
    const auto iterator = std::find_if(bindings_.begin(), bindings_.end(), [&](const Binding& current) { return current.logicalExecutorId == binding.logicalExecutorId; });
    if (iterator == bindings_.end()) return "not_found";
    if (iterator->version != expectedVersion) return "binding_version_conflict";
    Binding updated = binding; updated.version = iterator->version + 1; updated.updatedAt = nowMs(); *iterator = std::move(updated);
    if (const auto result = writeBindings(); !result.empty()) return result;
    return appendEvent("binding_updated", operatorName + " 更新了 " + binding.logicalExecutorId + " 绑定。");
}
const std::vector<RunSummary>& MarineRepository::listRunSummaries() const { return runs_; }
std::string MarineRepository::saveRunSummaries(const std::vector<RunSummary>& runs) { runs_ = runs; return writeRuns(); }
std::string MarineRepository::appendEvent(const std::string& type, const std::string& message) { std::ofstream stream(fs::path(root_) / "events.jsonl", std::ios::app); if (!stream) return "无法追加事件。"; stream << toJson(JsonValue::Object{{"type", type}, {"message", message}, {"at", static_cast<double>(nowMs())}}) << '\n'; return stream.good() ? "" : "无法追加事件。"; }

std::string MarineRepository::readApps() { const fs::path path = fs::path(root_) / "apps.json"; if (!fs::exists(path)) return ""; std::string text, error; if (!(error = readFile(path, text)).empty()) return error; JsonValue root; if (!parseJson(text, root, error) || !root.isArray()) return "apps.json 格式错误。"; apps_.clear(); for (const auto& item : root.asArray()) { Application app; if (!applicationFromJson(item, app, error)) return "apps.json 包含无效应用。"; apps_.push_back(std::move(app)); } return ""; }
std::string MarineRepository::readBindings() { const fs::path path = fs::path(root_) / "bindings.json"; if (!fs::exists(path)) { bindings_ = defaultBindings(); return writeBindings(); } std::string text, error; if (!(error = readFile(path, text)).empty()) return error; JsonValue root; if (!parseJson(text, root, error) || !root.isArray()) return "bindings.json 格式错误。"; bindings_.clear(); for (const auto& item : root.asArray()) { Binding binding; if (!bindingFromJson(item, binding)) return "bindings.json 包含无效绑定。"; bindings_.push_back(std::move(binding)); } return ""; }
std::string MarineRepository::readRuns() { const fs::path path = fs::path(root_) / "runs.json"; if (!fs::exists(path)) return ""; std::string text, error; if (!(error = readFile(path, text)).empty()) return error; JsonValue root; if (!parseJson(text, root, error) || !root.isArray()) return "runs.json 格式错误。"; runs_.clear(); for (const auto& item : root.asArray()) { RunSummary run; if (!runFromJson(item, run)) return "runs.json 包含无效任务。"; runs_.push_back(std::move(run)); } return ""; }
std::string MarineRepository::writeApps() { JsonValue::Array values; for (const auto& app : apps_) values.push_back(applicationToJson(app)); return writeAtomically(fs::path(root_) / "apps.json", toJson(JsonValue(std::move(values)))); }
std::string MarineRepository::writeBindings() { JsonValue::Array values; for (const auto& binding : bindings_) values.push_back(bindingToJson(binding)); return writeAtomically(fs::path(root_) / "bindings.json", toJson(JsonValue(std::move(values)))); }
std::string MarineRepository::writeRuns() { JsonValue::Array values; for (const auto& run : runs_) values.push_back(runToJson(run)); return writeAtomically(fs::path(root_) / "runs.json", toJson(JsonValue(std::move(values)))); }

std::string runStatusToString(RunStatus status) { switch (status) { case RunStatus::Idle: return "idle"; case RunStatus::Running: return "running"; case RunStatus::Paused: return "paused"; case RunStatus::Completed: return "completed"; case RunStatus::Cancelled: return "cancelled"; case RunStatus::Failed: return "failed"; case RunStatus::Interrupted: return "interrupted"; } return "idle"; }
bool runStatusFromString(const std::string& value, RunStatus& status) { const std::vector<std::pair<std::string, RunStatus>> values = {{"idle", RunStatus::Idle}, {"running", RunStatus::Running}, {"paused", RunStatus::Paused}, {"completed", RunStatus::Completed}, {"cancelled", RunStatus::Cancelled}, {"failed", RunStatus::Failed}, {"interrupted", RunStatus::Interrupted}}; const auto iterator = std::find_if(values.begin(), values.end(), [&](const auto& entry) { return entry.first == value; }); if (iterator == values.end()) return false; status = iterator->second; return true; }

} // namespace marine
