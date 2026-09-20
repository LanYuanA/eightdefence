#include "marine_api.hpp"

#include <algorithm>
#include <chrono>

namespace marine {
namespace {
uint64_t nowMs() { return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()); }
HttpResponse respond(const JsonValue& value, int code = 200) { HttpResponse response; response.statusCode = code; response.statusText = code == 201 ? "Created" : code == 204 ? "No Content" : code == 200 ? "OK" : "Error"; response.contentType = "application/json; charset=utf-8"; response.body = code == 204 ? "" : toJson(value); return response; }
HttpResponse failure(int code, const std::string& name, const std::string& message) { return respond(JsonValue::Object{{"error", JsonValue::Object{{"code", name}, {"message", message}}}}, code); }
const JsonValue* field(const JsonValue::Object& object, const std::string& name) { const auto it = object.find(name); return it == object.end() ? nullptr : &it->second; }
bool stringField(const JsonValue::Object& object, const std::string& name, std::string& value) { const auto item = field(object, name); if (item == nullptr || !item->isString() || item->asString().empty()) return false; value = item->asString(); return true; }
bool intField(const JsonValue::Object& object, const std::string& name, int& value) { const auto item = field(object, name); if (item == nullptr || !item->isNumber()) return false; value = static_cast<int>(item->asNumber()); return item->asNumber() == value; }
bool objectBody(const HttpRequest& request, JsonValue::Object& object, HttpResponse& invalid) { JsonValue value; std::string error; if (!parseJson(request.body, value, error) || !value.isObject()) { invalid = failure(400, "invalid_json", "请求正文必须是 JSON 对象。"); return false; } object = value.asObject(); return true; }
JsonValue bindingJson(const Binding& value) { JsonValue::Array capabilities; for (const auto& item : value.capabilities) capabilities.emplace_back(item); return JsonValue::Object{{"logicalExecutorId", value.logicalExecutorId}, {"deviceId", value.deviceId}, {"bus", value.bus}, {"address", value.address}, {"capabilities", std::move(capabilities)}, {"version", static_cast<double>(value.version)}, {"updatedAt", static_cast<double>(value.updatedAt)}, {"realControlEnabled", value.realControlEnabled}}; }
JsonValue runJson(const MarineRun& run) { JsonValue::Object result{{"id", run.id}, {"appId", run.appId}, {"status", runStatusToString(run.status)}, {"stepIndex", run.stepIndex}, {"elapsed", static_cast<double>(run.elapsedMs) / 1000}}; JsonValue::Object nodes; for (const auto& entry : run.nodes) nodes.emplace(entry.first, JsonValue::Object{{"status", nodeStatusToString(entry.second.status)}, {"elapsed", static_cast<double>(entry.second.elapsedMs) / 1000}, {"progress", entry.second.progress}, {"result", JsonValue::Object{{"metric", entry.second.result.metric}, {"value", entry.second.result.value}, {"unit", entry.second.result.unit}, {"detail", entry.second.result.detail}, {"source", entry.second.result.source}}}}); JsonValue::Array events; for (const auto& event : run.events) events.emplace_back(JsonValue::Object{{"time", static_cast<double>(event.atMs) / 1000}, {"message", event.message}, {"kind", event.kind}}); result.emplace("nodes", std::move(nodes)); result.emplace("events", std::move(events)); return result; }
std::vector<std::string> capabilitiesFor(const std::string& device) { if (device == "MOTOR-02") return {"ventilation", "speed", "status"}; if (device == "MOTOR-0E") return {"cooling", "water", "drainage", "status"}; if (device == "MOTOR-0F") return {"ventilation", "cooling", "drainage", "speed", "status"}; return {}; }
std::string addressFor(const std::string& device) { return device == "MOTOR-02" ? "0x02" : device == "MOTOR-0E" ? "0x0E" : device == "MOTOR-0F" ? "0x0F" : ""; }
bool compatible(const std::vector<std::string>& available, const std::vector<std::string>& required) { return std::all_of(required.begin(), required.end(), [&](const std::string& item) { return std::find(available.begin(), available.end(), item) != available.end(); }); }
JsonValue motorJson(const MotorTelemetry& motor) { return JsonValue::Object{{"executorId", motor.executorId}, {"name", motor.name}, {"model", motor.model}, {"address", motor.address}, {"owner", motor.owner}, {"online", motor.online}, {"running", motor.running}, {"targetRpm", motor.targetRpm}, {"actualRpm", motor.actualRpm}, {"direction", motor.direction}, {"statusWord", static_cast<double>(motor.statusWord)}, {"source", motor.source}}; }
std::string executorForDevice(const std::string& device) { return device == "MOTOR-02" ? "EXHAUST-FAN-01" : device == "MOTOR-0E" ? "FIRE-PUMP-01" : device == "MOTOR-0F" ? "DRAIN-PUMP-01" : ""; }
}

MarineApi::MarineApi(MarineRepository& repository, MarineRuntime& runtime, MarineSafety& safety, std::shared_ptr<MotorAtomicService> motors) : repository_(repository), runtime_(runtime), safety_(safety), motors_(std::move(motors)) {}
HttpResponse MarineApi::handle(const HttpRequest& request) {
    const auto& path = request.path;
    if (request.method == "GET" && path == "/api/v1/health") { const auto status = safety_.status(nowMs()); return respond(JsonValue::Object{{"version", "v1"}, {"storage", "ready"}, {"sensorMode", "demo"}, {"motorMode", motors_ && !motors_->simulation() ? "real" : "simulation"}, {"actuatorControlEnabled", status.actuatorControlEnabled}, {"emergencyStopped", status.emergencyStopped}, {"locked", !status.unlocked}}); }
    if (request.method == "GET" && path == "/api/v1/marine/motors") { if (!motors_) return failure(503, "motor_service_unavailable", "电机原子服务未初始化。"); JsonValue::Array values; for (const auto& motor : motors_->list()) values.emplace_back(motorJson(motor)); return respond(values); }
    const std::string hardwareDemoPrefix = "/api/v1/marine/hardware-demo/";
    if (request.method == "POST" && path.rfind(hardwareDemoPrefix, 0) == 0) {
        if (!motors_) return failure(503, "motor_service_unavailable", "电机原子服务未初始化。");
        JsonValue::Object body; HttpResponse invalid;
        if (!objectBody(request, body, invalid)) return invalid;
        std::string operatorName;
        if (!stringField(body, "operator", operatorName)) return failure(400, "validation_failed", "缺少操作员名称。");
        if (!safety_.status(nowMs()).actuatorControlEnabled) return failure(423, "emergency_stopped", "执行器急停尚未复位。");
        auto binding = std::find_if(repository_.listBindings().begin(), repository_.listBindings().end(), [](const Binding& value) { return value.logicalExecutorId == "COOL-01"; });
        if (binding == repository_.listBindings().end()) return failure(404, "binding_not_found", "中央冷却泵逻辑绑定不存在。");
        const std::string action = path.substr(hardwareDemoPrefix.size());
        std::string targetDevice = binding->deviceId;
        int speedRpm = 200;
        MotorDirection direction = MotorDirection::Forward;

        if (action == "reset") {
            const auto inventory = motors_->list();
            const auto online = std::find_if(inventory.begin(), inventory.end(), [](const MotorTelemetry& motor) { return motor.online; });
            if (online == inventory.end()) return failure(409, "no_motor_online", "尚未检测到接入的电机，请等待心跳扫描。");
            targetDevice = online->executorId == "EXHAUST-FAN-01" ? "MOTOR-02" : online->executorId == "FIRE-PUMP-01" ? "MOTOR-0E" : "MOTOR-0F";
        } else if (action == "fault") {
            const auto executor = executorForDevice(targetDevice);
            if (executor.empty()) return failure(409, "invalid_binding", "当前绑定设备无对应电机。");
            const auto outcome = motors_->stop("", executor);
            if (!outcome.empty()) return failure(502, outcome, "模拟故障时停止电机失败。");
            return respond(JsonValue::Object{{"action", action}, {"binding", bindingJson(*binding)}, {"motor", motorJson(motors_->telemetry(executor))}});
        } else if (action == "switch") {
            std::string directionText;
            if (!stringField(body, "deviceId", targetDevice) || executorForDevice(targetDevice).empty() || !intField(body, "speedRpm", speedRpm) || speedRpm < 1 || speedRpm > 500 || !stringField(body, "direction", directionText) || (directionText != "forward" && directionText != "reverse")) return failure(400, "validation_failed", "替换设备或运行参数无效。");
            direction = directionText == "reverse" ? MotorDirection::Reverse : MotorDirection::Forward;
            if (targetDevice == binding->deviceId) return failure(409, "same_device", "请选择不同的替换电机。");
        } else return failure(404, "not_found", "接口不存在。");

        const std::string targetExecutor = executorForDevice(targetDevice);
        motors_->stop("", targetExecutor);
        MotorParameters parameters; parameters.executorId = targetExecutor; parameters.speedRpm = speedRpm; parameters.direction = direction; parameters.acceleration = 10; parameters.deceleration = 10;
        const auto started = motors_->start("HARDWARE-DECOUPLING", parameters);
        if (!started.empty()) return failure(started == "device_error" ? 502 : 409, started, "替换电机启动失败。");

        Binding updated = *binding; updated.deviceId = targetDevice; updated.address = addressFor(targetDevice); updated.bus = "RS485";
        const auto saved = repository_.updateBinding(updated, binding->version, operatorName);
        if (!saved.empty()) { motors_->stop("", targetExecutor); return failure(500, saved, "逻辑执行器绑定更新失败。"); }
        binding = std::find_if(repository_.listBindings().begin(), repository_.listBindings().end(), [](const Binding& value) { return value.logicalExecutorId == "COOL-01"; });
        return respond(JsonValue::Object{{"action", action}, {"binding", bindingJson(*binding)}, {"motor", motorJson(motors_->telemetry(targetExecutor))}});
    }
    const std::string softwareDemoPrefix = "/api/v1/marine/software-demo/";
    if (request.method == "POST" && path.rfind(softwareDemoPrefix, 0) == 0) {
        if (!motors_) return failure(503, "motor_service_unavailable", "电机原子服务未初始化。");
        JsonValue::Object body; HttpResponse invalid;
        if (!objectBody(request, body, invalid)) return invalid;
        std::string operatorName;
        if (!stringField(body, "operator", operatorName)) return failure(400, "validation_failed", "缺少操作员名称。");
        if (!safety_.status(nowMs()).actuatorControlEnabled) return failure(423, "emergency_stopped", "执行器急停尚未复位。");
        const std::array<std::string, 3> executors{{"EXHAUST-FAN-01", "FIRE-PUMP-01", "DRAIN-PUMP-01"}};
        const std::string action = path.substr(softwareDemoPrefix.size());
        std::vector<int> targets;
        if (action == "execute") {
            const auto numbers = field(body, "motorNumbers");
            if (numbers == nullptr || !numbers->isArray()) return failure(400, "validation_failed", "缺少电机编号列表。");
            for (const auto& value : numbers->asArray()) {
                if (!value.isNumber() || value.asNumber() != static_cast<int>(value.asNumber())) return failure(400, "validation_failed", "电机编号必须为整数。");
                const int number = static_cast<int>(value.asNumber());
                if (number >= 1 && number <= 3 && std::find(targets.begin(), targets.end(), number) == targets.end()) targets.push_back(number);
            }
        } else if (action == "reset" || action == "stop") targets = {1, 2, 3};
        else return failure(404, "not_found", "接口不存在。");

        // 真实串口模式下一次只会绑定一台电机。场景中的其余编号由前端模拟，
        // 后端不得继续向未接入地址发送控制帧，否则一次正常演示会被离线设备拖失败。
        if (!motors_->simulation()) {
            const auto inventory = motors_->list();
            std::vector<int> onlineNumbers;
            for (size_t index = 0; index < executors.size(); ++index) {
                const auto found = std::find_if(inventory.begin(), inventory.end(), [&](const MotorTelemetry& motor) {
                    return motor.executorId == executors.at(index) && motor.online;
                });
                if (found != inventory.end()) onlineNumbers.push_back(static_cast<int>(index + 1));
            }
            targets.erase(std::remove_if(targets.begin(), targets.end(), [&](int number) {
                return std::find(onlineNumbers.begin(), onlineNumbers.end(), number) == onlineNumbers.end();
            }), targets.end());
        }

        JsonValue::Array controlled;
        JsonValue::Array telemetry;
        for (const int number : targets) {
            const auto& executor = executors.at(static_cast<size_t>(number - 1));
            std::string outcome;
            if (action == "reset") {
                motors_->stop("", executor);
                MotorParameters parameters; parameters.executorId = executor; parameters.speedRpm = 200; parameters.direction = MotorDirection::Forward; parameters.acceleration = 10; parameters.deceleration = 10;
                outcome = motors_->start("SOFTWARE-DECOUPLING", parameters);
            } else outcome = motors_->stop("", executor);
            if (!outcome.empty()) return failure(outcome == "device_error" ? 502 : 409, outcome, "场景电机控制失败：电机" + std::to_string(number));
            controlled.emplace_back(number);
            telemetry.emplace_back(motorJson(motors_->telemetry(executor)));
        }
        return respond(JsonValue::Object{{"action", action}, {"controlledMotorNumbers", std::move(controlled)}, {"motors", std::move(telemetry)}});
    }
    const std::string motorPrefix = "/api/v1/marine/motors/";
    if (request.method == "POST" && path.rfind(motorPrefix, 0) == 0) {
        if (!motors_) return failure(503, "motor_service_unavailable", "电机原子服务未初始化。");
        const auto actionPos = path.find('/', motorPrefix.size());
        if (actionPos != std::string::npos) {
            const std::string executorId = path.substr(motorPrefix.size(), actionPos - motorPrefix.size());
            const std::string action = path.substr(actionPos + 1);
            JsonValue::Object body; HttpResponse invalid;
            if (!objectBody(request, body, invalid)) return invalid;
            std::string operatorName;
            if (!stringField(body, "operator", operatorName)) return failure(400, "validation_failed", "缺少操作员名称。");
            if (!safety_.status(nowMs()).actuatorControlEnabled) return failure(423, "emergency_stopped", "执行器急停尚未复位。");
            std::string outcome;
            if (action == "start") {
                MotorParameters parameters; parameters.executorId = executorId; std::string direction;
                if (!intField(body, "speedRpm", parameters.speedRpm) || !stringField(body, "direction", direction) || !intField(body, "acceleration", parameters.acceleration) || !intField(body, "deceleration", parameters.deceleration) || parameters.speedRpm < 1 || parameters.speedRpm > 500 || parameters.acceleration < 1 || parameters.acceleration > 100 || parameters.deceleration < 1 || parameters.deceleration > 100 || (direction != "forward" && direction != "reverse")) return failure(400, "validation_failed", "电机参数不完整或超出范围。");
                parameters.direction = direction == "reverse" ? MotorDirection::Reverse : MotorDirection::Forward;
                outcome = motors_->start("HARDWARE-DECOUPLING", parameters);
                if (outcome == "resource_busy") outcome = motors_->update("HARDWARE-DECOUPLING", parameters);
            } else if (action == "stop") {
                outcome = motors_->stop("HARDWARE-DECOUPLING", executorId);
            } else return failure(404, "not_found", "接口不存在。");
            if (!outcome.empty()) return failure(outcome == "device_error" ? 502 : outcome == "unknown_executor" ? 404 : 409, outcome, "电机控制失败。");
            return respond(motorJson(motors_->telemetry(executorId)));
        }
    }
    if (request.method == "POST" && (path == "/api/v1/marine/emergency-stop" || path == "/api/v1/marine/emergency-reset")) { if (!motors_) return failure(503, "motor_service_unavailable", "电机原子服务未初始化。"); JsonValue::Object body; HttpResponse invalid; if (!objectBody(request, body, invalid)) return invalid; std::string operatorName; if (!stringField(body, "operator", operatorName)) return failure(400, "validation_failed", "缺少操作员名称。"); if (path.find("reset") != std::string::npos) { motors_->resetEmergency(); safety_.resetEmergency(operatorName, nowMs()); } else { const bool stopped = runtime_.emergencyStopMotors(); safety_.emergencyStop(operatorName, nowMs()); if (!stopped) return failure(502, "emergency_stop_incomplete", "急停已锁定，但至少一台电机未确认停止，请立即检查现场。"); } const auto status = safety_.status(nowMs()); return respond(JsonValue::Object{{"actuatorControlEnabled", status.actuatorControlEnabled}, {"emergencyStopped", status.emergencyStopped}}); }
    if (request.method == "GET" && path == "/api/v1/marine/apps") { JsonValue::Array apps; for (const auto& app : repository_.listApps()) apps.push_back(applicationToJson(app)); return respond(apps); }
    if (request.method == "POST" && path == "/api/v1/marine/apps") { JsonValue body; std::string issue; Application app; if (!parseJson(request.body, body, issue) || !applicationFromJson(body, app, issue)) return failure(400, "validation_failed", issue.empty() ? "应用定义无效。" : issue); const auto saved = repository_.saveApp(app); return saved.empty() ? respond(applicationToJson(app), 201) : failure(400, "validation_failed", saved); }

    const std::string appPrefix = "/api/v1/marine/apps/";
    if (path.rfind(appPrefix, 0) == 0) { const std::string id = path.substr(appPrefix.size()); auto found = std::find_if(repository_.listApps().begin(), repository_.listApps().end(), [&](const Application& app) { return app.id == id; }); if (request.method == "GET") return found == repository_.listApps().end() ? failure(404, "not_found", "应用不存在。") : respond(applicationToJson(*found)); if (request.method == "PUT") { JsonValue body; std::string issue; Application app; if (!parseJson(request.body, body, issue) || !applicationFromJson(body, app, issue) || app.id != id) return failure(400, "validation_failed", "应用定义或 ID 无效。"); const auto saved = repository_.saveApp(app); return saved.empty() ? respond(applicationToJson(app)) : failure(400, "validation_failed", saved); } if (request.method == "DELETE") { for (const auto& run : runtime_.listRuns()) if (run.appId == id && (run.status == RunStatus::Running || run.status == RunStatus::Paused)) return failure(409, "run_active", "应用仍有运行实例。"); const auto removed = repository_.removeApp(id); return removed.empty() ? respond(JsonValue(), 204) : failure(removed == "not_found" ? 404 : 500, removed, removed == "not_found" ? "应用不存在。" : "删除应用失败。"); } }

    if (request.method == "POST" && path == "/api/v1/marine/runs") { JsonValue::Object body; HttpResponse invalid; if (!objectBody(request, body, invalid)) return invalid; std::string appId; if (!stringField(body, "appId", appId)) return failure(400, "validation_failed", "缺少 appId。"); const auto run = runtime_.createRun(appId); if (run.id.empty()) return failure(404, "not_found", "应用不存在。"); if (run.status == RunStatus::Failed) return failure(409, "run_start_failed", run.events.empty() ? "应用启动失败。" : run.events.back().message); return respond(runJson(run), 201); }
    if (request.method == "GET" && path == "/api/v1/marine/runs") { JsonValue::Array runs; for (const auto& run : runtime_.listRuns()) runs.push_back(runJson(run)); return respond(runs); }
    const std::string runPrefix = "/api/v1/marine/runs/";
    if (path.rfind(runPrefix, 0) == 0) {
        const std::string rest = path.substr(runPrefix.size()); const auto nodesAt = rest.find("/nodes/");
        if (nodesAt != std::string::npos) { const std::string runId = rest.substr(0, nodesAt); const std::string tail = rest.substr(nodesAt + 7); const auto slash = tail.find('/'); const std::string nodeId = tail.substr(0, slash); const std::string action = slash == std::string::npos ? "" : tail.substr(slash + 1);
            if (request.method == "PATCH" && action == "motor") { JsonValue::Object body; HttpResponse invalid; if (!objectBody(request, body, invalid)) return invalid; MotorParameters parameters; std::string direction; if (!stringField(body, "executorId", parameters.executorId) || !intField(body, "speedRpm", parameters.speedRpm) || !stringField(body, "direction", direction) || !intField(body, "acceleration", parameters.acceleration) || !intField(body, "deceleration", parameters.deceleration) || parameters.speedRpm < 1 || parameters.speedRpm > 500 || parameters.acceleration < 1 || parameters.acceleration > 100 || parameters.deceleration < 1 || parameters.deceleration > 100 || (direction != "forward" && direction != "reverse")) return failure(400, "validation_failed", "电机参数不完整或超出范围。"); parameters.direction = direction == "reverse" ? MotorDirection::Reverse : MotorDirection::Forward; const auto outcome = runtime_.updateMotor(runId, nodeId, parameters); if (outcome == "not_found" || outcome == "node_not_found") return failure(404, outcome, "运行实例或节点不存在。"); if (outcome == "emergency_stopped") return failure(423, outcome, "执行器急停尚未复位。"); if (!outcome.empty()) return failure(outcome == "device_error" ? 502 : 409, outcome, "电机参数更新失败。"); return respond(runJson(*runtime_.getRun(runId))); }
            if (request.method == "POST" && action == "stop") { const auto outcome = runtime_.stopMotor(runId, nodeId); if (outcome == "not_found" || outcome == "node_not_found") return failure(404, outcome, "运行实例或节点不存在。"); if (!outcome.empty()) return failure(outcome == "device_error" ? 502 : 409, outcome, "电机停止失败。"); return respond(runJson(*runtime_.getRun(runId))); }
        }
    }
    if (path.rfind(runPrefix, 0) == 0) { const std::string rest = path.substr(runPrefix.size()); const auto slash = rest.find('/'); const std::string id = rest.substr(0, slash); const std::string action = slash == std::string::npos ? "" : rest.substr(slash + 1); if (request.method == "GET" && action.empty()) { const auto run = runtime_.getRun(id); return run ? respond(runJson(*run)) : failure(404, "not_found", "运行实例不存在。"); } if (request.method == "POST" && (action == "pause" || action == "resume" || action == "cancel")) { const auto outcome = action == "pause" ? runtime_.pauseRun(id) : action == "resume" ? runtime_.resumeRun(id) : runtime_.cancelRun(id); if (outcome == "not_found") return failure(404, outcome, "运行实例不存在。"); if (!outcome.empty()) return failure(409, outcome, "当前状态不允许该操作。"); return respond(runJson(*runtime_.getRun(id))); } }

    if (request.method == "GET" && path == "/api/v1/marine/executors") { JsonValue::Array values; for (const auto& binding : repository_.listBindings()) values.push_back(bindingJson(binding)); return respond(values); }
    const std::string executorPrefix = "/api/v1/marine/executors/";
    if (request.method == "PUT" && path.rfind(executorPrefix, 0) == 0 && path.size() > executorPrefix.size() + 8 && path.substr(path.size() - 8) == "/binding") { const std::string id = path.substr(executorPrefix.size(), path.size() - executorPrefix.size() - 8); JsonValue::Object body; HttpResponse invalid; if (!objectBody(request, body, invalid)) return invalid; std::string device, operatorName; const auto expected = field(body, "expectedVersion"); if (!stringField(body, "deviceId", device) || !stringField(body, "operator", operatorName) || expected == nullptr || !expected->isNumber() || expected->asNumber() < 0) return failure(400, "validation_failed", "绑定请求参数不完整。"); const uint64_t version = static_cast<uint64_t>(expected->asNumber()); auto current = std::find_if(repository_.listBindings().begin(), repository_.listBindings().end(), [&](const Binding& binding) { return binding.logicalExecutorId == id; }); if (current == repository_.listBindings().end()) return failure(404, "not_found", "逻辑执行器不存在。"); if (current->version != version) return failure(409, "binding_version_conflict", "设备绑定版本已变化，请重新读取。"); const auto capabilities = capabilitiesFor(device); if (capabilities.empty() || !compatible(capabilities, current->capabilities)) return failure(409, "capability_mismatch", "候选设备不具备所需能力。"); Binding updated = *current; updated.deviceId = device; updated.bus = "RS485"; updated.address = addressFor(device); updated.capabilities = capabilities; const auto saved = repository_.updateBinding(updated, version, operatorName); if (!saved.empty()) return failure(saved == "binding_version_conflict" ? 409 : 500, saved, "绑定更新失败。"); current = std::find_if(repository_.listBindings().begin(), repository_.listBindings().end(), [&](const Binding& binding) { return binding.logicalExecutorId == id; }); return respond(bindingJson(*current)); }

    if (request.method == "GET" && path == "/api/v1/safety") { const auto status = safety_.status(nowMs()); return respond(JsonValue::Object{{"actuatorControlEnabled", status.actuatorControlEnabled}, {"unlocked", status.unlocked}, {"remainingSeconds", static_cast<double>(status.remainingSeconds)}}); }
    if (request.method == "POST" && (path == "/api/v1/safety/unlock" || path == "/api/v1/safety/lock")) { JsonValue::Object body; HttpResponse invalid; if (!objectBody(request, body, invalid)) return invalid; std::string operatorName; if (!stringField(body, "operator", operatorName)) return failure(400, "validation_failed", "缺少操作员名称。"); const auto status = path == "/api/v1/safety/unlock" ? safety_.unlock(operatorName, nowMs()) : safety_.lock(operatorName, nowMs()); return respond(JsonValue::Object{{"actuatorControlEnabled", status.actuatorControlEnabled}, {"unlocked", status.unlocked}, {"remainingSeconds", static_cast<double>(status.remainingSeconds)}}); }
    return failure(404, "not_found", "接口不存在。");
}
} // namespace marine
