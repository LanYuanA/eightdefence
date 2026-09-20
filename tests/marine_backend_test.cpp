#include "application/marine/json_value.hpp"
#include "application/marine/marine_types.hpp"
#include "application/marine/marine_repository.hpp"
#include "application/marine/marine_safety.hpp"
#include "application/marine/marine_executor.hpp"
#include "application/marine/marine_runtime.hpp"
#include "application/marine/marine_api.hpp"
#include "service/atomic/svc_motor_speed.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <atomic>
#include <future>
#include <thread>

namespace {
int failures = 0;

#define REQUIRE(condition) do { \
    if (!(condition)) { \
        std::cerr << "失败: " << #condition << " (" << __FILE__ << ':' << __LINE__ << ")\n"; \
        ++failures; \
    } \
} while (false)

marine::Application makeApplication(const std::vector<std::vector<std::string>>& serviceIds) {
    marine::Application app;
    app.id = "APP-01";
    app.name = "测试应用";
    app.description = "用于领域校验";
    int stepIndex = 0;
    for (const auto& services : serviceIds) {
        marine::TaskStep step;
        step.id = "STEP-" + std::to_string(++stepIndex);
        int nodeIndex = 0;
        for (const auto& serviceId : services) {
            step.nodes.push_back({"NODE-" + std::to_string(stepIndex) + '-' + std::to_string(++nodeIndex), serviceId, "机舱", 70, 6});
        }
        app.steps.push_back(step);
    }
    return app;
}

class TempDir {
public:
    TempDir() : path_(std::filesystem::temp_directory_path() / ("marine-backend-test-" + std::to_string(++next_))) {
        std::filesystem::remove_all(path_);
        std::filesystem::create_directories(path_);
    }
    ~TempDir() { std::filesystem::remove_all(path_); }
    std::string path() const { return path_.string(); }
private:
    std::filesystem::path path_;
    static int next_;
};
int TempDir::next_ = 0;

void application_validation_rejects_same_resource_in_parallel_step() {
    const auto errors = marine::validateApplication(makeApplication({{"03", "04"}}));
    REQUIRE(!errors.empty());
    REQUIRE(errors.front() == "步骤 1 的服务共用水务执行器，请拆成顺序步骤。");
}

void application_json_round_trip_preserves_parallel_nodes() {
    auto input = makeApplication({{"A01"}, {"02", "03"}});
    input.steps.at(0).nodes.at(0).threshold = 26;
    input.steps.at(0).nodes.at(0).thresholdOperator = "gte";
    marine::JsonValue value;
    std::string error;
    REQUIRE(marine::parseJson(marine::toJson(marine::applicationToJson(input)), value, error));
    marine::Application output;
    REQUIRE(marine::applicationFromJson(value, output, error));
    REQUIRE(output.steps.size() == 2);
    REQUIRE(output.steps.at(1).nodes.size() == 2);
    REQUIRE(output.steps.at(1).nodes.at(1).serviceId == "03");
    REQUIRE(output.steps.at(0).nodes.at(0).threshold == 26);
    REQUIRE(output.steps.at(0).nodes.at(0).thresholdOperator == "gte");
}

void motor_parameters_round_trip_and_validate_boundaries() {
    auto input = makeApplication({{"M01"}});
    auto& motor = input.steps.at(0).nodes.at(0).motor;
    motor.executorId = "EXHAUST-FAN-01";
    motor.speedRpm = 200;
    motor.direction = marine::MotorDirection::Reverse;
    motor.acceleration = 10;
    motor.deceleration = 10;
    marine::JsonValue value;
    std::string error;
    REQUIRE(marine::parseJson(marine::toJson(marine::applicationToJson(input)), value, error));
    marine::Application output;
    REQUIRE(marine::applicationFromJson(value, output, error));
    const auto& restored = output.steps.at(0).nodes.at(0).motor;
    REQUIRE(restored.executorId == "EXHAUST-FAN-01");
    REQUIRE(restored.speedRpm == 200);
    REQUIRE(restored.direction == marine::MotorDirection::Reverse);
    REQUIRE(restored.acceleration == 10);
    REQUIRE(restored.deceleration == 10);

    input.steps.at(0).nodes.at(0).motor.speedRpm = 0;
    REQUIRE(!marine::validateApplication(input).empty());
    input.steps.at(0).nodes.at(0).motor.speedRpm = 501;
    REQUIRE(!marine::validateApplication(input).empty());
    input.steps.at(0).nodes.at(0).motor.speedRpm = 200;
    input.steps.at(0).nodes.at(0).motor.executorId = "UNKNOWN";
    REQUIRE(!marine::validateApplication(input).empty());
}

void json_parser_rejects_trailing_content() {
    marine::JsonValue value;
    std::string error;
    REQUIRE(!marine::parseJson("{\"id\":\"APP-01\"} extra", value, error));
}

void repository_persists_application_across_reopen() {
    TempDir data;
    marine::MarineRepository first(data.path());
    REQUIRE(first.load().empty());
    const auto app = makeApplication({{"A01"}, {"01"}});
    REQUIRE(first.saveApp(app).empty());
    marine::MarineRepository reopened(data.path());
    REQUIRE(reopened.load().empty());
    REQUIRE(reopened.listApps().size() == 1);
    REQUIRE(reopened.listApps().at(0).id == "APP-01");
}

void binding_update_rejects_stale_version_without_mutation() {
    TempDir data;
    marine::MarineRepository repository(data.path());
    REQUIRE(repository.load().empty());
    const auto before = repository.listBindings().at(0);
    auto candidate = before;
    candidate.deviceId = "MOTOR-0E";
    candidate.address = "0x0E";
    REQUIRE(repository.updateBinding(candidate, before.version - 1, "演示员") == "binding_version_conflict");
    REQUIRE(repository.listBindings().at(0).deviceId == before.deviceId);
}

void repository_marks_unfinished_runs_interrupted_on_load() {
    TempDir data;
    marine::MarineRepository first(data.path());
    REQUIRE(first.load().empty());
    REQUIRE(first.saveRunSummaries({{"RUN-01", "APP-01", marine::RunStatus::Running, 0}}).empty());
    marine::MarineRepository reopened(data.path());
    REQUIRE(reopened.load().empty());
    REQUIRE(reopened.listRunSummaries().at(0).status == marine::RunStatus::Interrupted);
}

void safety_defaults_to_enabled_and_latches_emergency_stop() {
    TempDir data;
    marine::MarineSafety safety(data.path() + "/events.jsonl");
    REQUIRE(safety.status(1000).actuatorControlEnabled);
    safety.emergencyStop("演示员", 2000);
    REQUIRE(!safety.status(2000).actuatorControlEnabled);
    REQUIRE(safety.status(2000).emergencyStopped);
    safety.resetEmergency("演示员", 3000);
    REQUIRE(safety.status(3000).actuatorControlEnabled);
}

void executor_returns_simulated_result_without_command_callback() {
    marine::MarineExecutor executor([] { return marine::SensorSnapshot::demo(); });
    marine::TaskNode node{"NODE-02", "02", "机舱", 70, 6};
    const auto result = executor.execute(node, 0.5, executor.snapshot());
    REQUIRE(result.source == "simulated");
    REQUIRE(result.metric == "循环流量");
    REQUIRE(result.value == "33.6");
}

void offline_sensor_is_reported_as_demo_data() {
    marine::MarineExecutor executor([] { return marine::SensorSnapshot{}; });
    REQUIRE(executor.snapshot().source == "demo");
}

void motor_service_locks_resources_and_latches_emergency_stop() {
    marine::MotorAtomicService motors(true);
    marine::MotorParameters parameters;
    parameters.executorId = "EXHAUST-FAN-01";
    REQUIRE(motors.start("RUN-1", parameters).empty());
    REQUIRE(motors.start("RUN-2", parameters) == "resource_busy");
    REQUIRE(motors.telemetry(parameters.executorId).running);
    REQUIRE(motors.stop("RUN-1", parameters.executorId).empty());
    REQUIRE(motors.start("RUN-2", parameters).empty());
    motors.emergencyStop();
    REQUIRE(motors.emergencyStopped());
    REQUIRE(!motors.telemetry(parameters.executorId).running);
    REQUIRE(motors.start("RUN-3", parameters) == "emergency_stopped");
    motors.resetEmergency();
    REQUIRE(motors.start("RUN-3", parameters).empty());
}

void motor_service_reads_signed_32_bit_speed_and_reports_failed_emergency_stop() {
    marine::MotorAtomicService motors(true);
    motors.attachIo(
        [](uint8_t, uint16_t, const std::vector<uint16_t>&) { return true; },
        [](uint8_t, uint16_t reg, uint16_t count, std::vector<uint16_t>& values) {
            if (reg == 0x606C && count == 2) values = {0xFFFF, 0xFF38};
            else if (reg == 0x6041 && count == 1) values = {0x0027};
            else return false;
            return true;
        });
    marine::MotorParameters liveParameters; liveParameters.executorId = "EXHAUST-FAN-01";
    REQUIRE(motors.start("TELEMETRY-TEST", liveParameters).empty());
    const auto telemetry = motors.telemetry("EXHAUST-FAN-01");
    REQUIRE(telemetry.online);
    REQUIRE(telemetry.actualRpm == -200);

    marine::MotorAtomicService statusOnly(true);
    statusOnly.attachIo(
        [](uint8_t, uint16_t, const std::vector<uint16_t>&) { return true; },
        [](uint8_t, uint16_t reg, uint16_t, std::vector<uint16_t>& values) {
            if (reg != 0x6041) return false;
            values = {0x0027}; return true;
        });
    REQUIRE(statusOnly.telemetry("EXHAUST-FAN-01").online);

    marine::MotorAtomicService failing(true);
    failing.attachIo(
        [](uint8_t, uint16_t, const std::vector<uint16_t>&) { return false; },
        [](uint8_t, uint16_t, uint16_t, std::vector<uint16_t>&) { return false; });
    REQUIRE(!failing.emergencyStop());
    REQUIRE(failing.emergencyStopped());
    REQUIRE(!failing.telemetry("EXHAUST-FAN-01").online);
}

void motor_inventory_discovers_one_device_then_heartbeats_only_that_address() {
    marine::MotorAtomicService motors(true);
    std::vector<uint8_t> reads;
    uint8_t connectedAddress = 0x0E;
    motors.attachIo(
        [](uint8_t, uint16_t, const std::vector<uint16_t>&) { return true; },
        [&](uint8_t address, uint16_t reg, uint16_t, std::vector<uint16_t>& values) {
            if (reg != 0x6041) return false;
            reads.push_back(address);
            if (address != connectedAddress) return false;
            values = {0x0027}; return true;
        });

    auto inventory = motors.list();
    REQUIRE(!inventory.at(0).online && !inventory.at(1).online && !inventory.at(2).online);
    inventory = motors.list();
    REQUIRE(!inventory.at(0).online && inventory.at(1).online && !inventory.at(2).online);
    motors.list();
    REQUIRE((reads == std::vector<uint8_t>{0x02, 0x0E, 0x0E}));

    connectedAddress = 0x0F;
    inventory = motors.list();
    REQUIRE(!inventory.at(0).online && !inventory.at(1).online && !inventory.at(2).online);
    inventory = motors.list();
    REQUIRE(!inventory.at(0).online && !inventory.at(1).online && inventory.at(2).online);
    motors.list();
    REQUIRE((reads == std::vector<uint8_t>{0x02, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F}));
}

void motor_inventory_can_scan_all_addresses_without_changing_the_binding() {
    marine::MotorAtomicService motors(true);
    std::vector<uint8_t> reads;
    motors.attachIo(
        [](uint8_t, uint16_t, const std::vector<uint16_t>&) { return true; },
        [&](uint8_t address, uint16_t reg, uint16_t count, std::vector<uint16_t>& values) {
            reads.push_back(address);
            if (reg == 0x6041 && count == 1) { values = {0x0221}; return true; }
            if (reg == 0x606C && count == 2) { values = {0, static_cast<uint16_t>(address)}; return true; }
            return false;
        });

    const auto scan = motors.scanAll();
    REQUIRE(scan.size() == 3);
    REQUIRE(std::all_of(scan.begin(), scan.end(), [](const marine::MotorTelemetry& motor) { return motor.online; }));
    REQUIRE((reads == std::vector<uint8_t>{0x02, 0x02, 0x0E, 0x0E, 0x0F, 0x0F}));
}

void inventory_refreshes_live_speed_after_startup_and_direction_changes() {
    marine::MotorAtomicService motors(true);
    int32_t speed = 28;
    std::vector<uint8_t> speedAddresses;
    motors.attachIo(
        [](uint8_t, uint16_t, const std::vector<uint16_t>&) { return true; },
        [&](uint8_t address, uint16_t reg, uint16_t count, std::vector<uint16_t>& values) {
            if (address != 0x02) return false;
            if (reg == 0x6041 && count == 1) { values = {0x27}; return true; }
            if (reg == 0x606C && count == 2) {
                speedAddresses.push_back(address);
                const auto raw = static_cast<uint32_t>(speed);
                values = {static_cast<uint16_t>(raw >> 16), static_cast<uint16_t>(raw)};
                return true;
            }
            return false;
        });
    REQUIRE(motors.list().at(0).actualRpm == 28);
    speed = 200;
    REQUIRE(motors.list().at(0).actualRpm == 200);
    speed = -200;
    const auto reverse = motors.list().at(0);
    REQUIRE(reverse.actualRpm == -200);
    REQUIRE(reverse.running);
    REQUIRE(reverse.direction == "reverse");
    speed = 0;
    REQUIRE(!motors.list().at(0).running);
    REQUIRE((speedAddresses == std::vector<uint8_t>{0x02, 0x02, 0x02, 0x02}));
}

void emergency_stop_preempts_a_blocked_runtime_start() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto app = makeApplication({{"M01"}}); REQUIRE(repository.saveApp(app).empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    std::atomic<bool> writeEntered{false}; std::atomic<bool> releaseWrite{false};
    motors->attachIo(
        [&](uint8_t, uint16_t, const std::vector<uint16_t>&) { writeEntered = true; while (!releaseWrite.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1)); return true; },
        [](uint8_t, uint16_t, uint16_t, std::vector<uint16_t>&) { return false; },
        [&](const std::vector<uint8_t>& addresses) { releaseWrite = true; return std::vector<bool>(addresses.size(), true); });
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    auto start = std::async(std::launch::async, [&] { return runtime.createRun(app.id); });
    for (int attempt = 0; attempt < 200 && !writeEntered.load(); ++attempt) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(writeEntered.load());
    auto stop = std::async(std::launch::async, [&] { return runtime.emergencyStopMotors(); });
    REQUIRE(stop.wait_for(std::chrono::milliseconds(500)) == std::future_status::ready);
    REQUIRE(stop.get());
    REQUIRE(start.wait_for(std::chrono::milliseconds(500)) == std::future_status::ready);
    REQUIRE(start.get().status == marine::RunStatus::Failed);
}

void runtime_waits_for_all_parallel_nodes_before_next_step() {
    TempDir data;
    marine::MarineRepository repository(data.path());
    REQUIRE(repository.load().empty());
    auto app = makeApplication({{"02", "03"}, {"04"}});
    app.id = "APP-PARALLEL";
    REQUIRE(repository.saveApp(app).empty());
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }));
    const auto run = runtime.createRun(app.id);
    REQUIRE(run.status == marine::RunStatus::Running);
    runtime.advanceForTest(6000);
    const auto current = runtime.getRun(run.id);
    REQUIRE(current.has_value());
    REQUIRE(current->stepIndex == 1);
    REQUIRE(current->nodes.at("NODE-2-1").status == marine::NodeStatus::Running);
}

void cancelling_one_run_does_not_cancel_another_run() {
    TempDir data;
    marine::MarineRepository repository(data.path());
    REQUIRE(repository.load().empty());
    auto firstApp = makeApplication({{"01"}}); firstApp.id = "APP-FIRST";
    auto secondApp = makeApplication({{"02"}}); secondApp.id = "APP-SECOND";
    REQUIRE(repository.saveApp(firstApp).empty());
    REQUIRE(repository.saveApp(secondApp).empty());
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }));
    const auto first = runtime.createRun(firstApp.id);
    const auto second = runtime.createRun(secondApp.id);
    REQUIRE(runtime.cancelRun(first.id).empty());
    REQUIRE(runtime.getRun(first.id)->status == marine::RunStatus::Cancelled);
    REQUIRE(runtime.getRun(second.id)->status == marine::RunStatus::Running);
}

marine::HttpRequest request(const std::string& method, const std::string& path, const std::string& body = "") {
    marine::HttpRequest value;
    value.method = method;
    value.path = path;
    value.fullPath = path;
    value.body = body;
    return value;
}

void api_creates_lists_and_starts_persisted_application() {
    TempDir data;
    marine::MarineRepository repository(data.path());
    REQUIRE(repository.load().empty());
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }));
    marine::MarineSafety safety(data.path() + "/events.jsonl");
    marine::MarineApi api(repository, runtime, safety);
    const auto app = makeApplication({{"A01"}, {"01"}});
    const auto create = api.handle(request("POST", "/api/v1/marine/apps", marine::toJson(marine::applicationToJson(app))));
    REQUIRE(create.statusCode == 201);
    const auto start = api.handle(request("POST", "/api/v1/marine/runs", "{\"appId\":\"APP-01\"}"));
    REQUIRE(start.statusCode == 201);
    const auto listed = api.handle(request("GET", "/api/v1/marine/runs"));
    REQUIRE(listed.statusCode == 200);
    REQUIRE(listed.body.find("APP-01") != std::string::npos);
}

void api_returns_conflict_for_stale_binding_version() {
    TempDir data;
    marine::MarineRepository repository(data.path());
    REQUIRE(repository.load().empty());
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }));
    marine::MarineSafety safety(data.path() + "/events.jsonl");
    marine::MarineApi api(repository, runtime, safety);
    const auto response = api.handle(request("PUT", "/api/v1/marine/executors/VENT-01/binding", "{\"deviceId\":\"MOTOR-0F\",\"expectedVersion\":0,\"operator\":\"演示员\"}"));
    REQUIRE(response.statusCode == 409);
    REQUIRE(response.body.find("binding_version_conflict") != std::string::npos);
}

void api_runs_updates_and_emergency_stops_motor_service() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto app = makeApplication({{"M01"}}); REQUIRE(repository.saveApp(app).empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    marine::MarineSafety safety(data.path() + "/events.jsonl"); marine::MarineApi api(repository, runtime, safety, motors);
    const auto created = api.handle(request("POST", "/api/v1/marine/runs", "{\"appId\":\"APP-01\"}")); REQUIRE(created.statusCode == 201);
    REQUIRE(api.handle(request("GET", "/api/v1/marine/motors")).body.find("IDS57-R") != std::string::npos);
    const auto updated = api.handle(request("PATCH", "/api/v1/marine/runs/RUN-1/nodes/NODE-1-1/motor", "{\"executorId\":\"EXHAUST-FAN-01\",\"speedRpm\":300,\"direction\":\"reverse\",\"acceleration\":10,\"deceleration\":10}")); REQUIRE(updated.statusCode == 200);
    const auto invalid = api.handle(request("PATCH", "/api/v1/marine/runs/RUN-1/nodes/NODE-1-1/motor", "{\"executorId\":\"EXHAUST-FAN-01\",\"speedRpm\":501,\"direction\":\"forward\",\"acceleration\":10,\"deceleration\":10}")); REQUIRE(invalid.statusCode == 400);
    const auto manualStop = api.handle(request("POST", "/api/v1/marine/runs/RUN-1/nodes/NODE-1-1/stop", "{}")); REQUIRE(manualStop.statusCode == 200); REQUIRE(manualStop.body.find("cancelled") != std::string::npos);
    const auto stopped = api.handle(request("POST", "/api/v1/marine/emergency-stop", "{\"operator\":\"演示员\"}")); REQUIRE(stopped.statusCode == 200); REQUIRE(stopped.body.find("true") != std::string::npos);
    const auto reset = api.handle(request("POST", "/api/v1/marine/emergency-reset", "{\"operator\":\"演示员\"}")); REQUIRE(reset.statusCode == 200);
}

void api_directly_controls_motor_for_hardware_decoupling_screen() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    marine::MarineSafety safety(data.path() + "/events.jsonl"); marine::MarineApi api(repository, runtime, safety, motors);

    const auto started = api.handle(request("POST", "/api/v1/marine/motors/FIRE-PUMP-01/start", "{\"speedRpm\":300,\"direction\":\"reverse\",\"acceleration\":10,\"deceleration\":10,\"operator\":\"演示员\"}"));
    REQUIRE(started.statusCode == 200);
    REQUIRE(started.body.find("\"targetRpm\":-300") != std::string::npos);
    REQUIRE(started.body.find("\"running\":true") != std::string::npos);

    const auto invalid = api.handle(request("POST", "/api/v1/marine/motors/FIRE-PUMP-01/start", "{\"speedRpm\":501,\"direction\":\"forward\",\"acceleration\":10,\"deceleration\":10,\"operator\":\"演示员\"}"));
    REQUIRE(invalid.statusCode == 400);
    const auto stopped = api.handle(request("POST", "/api/v1/marine/motors/FIRE-PUMP-01/stop", "{\"operator\":\"演示员\"}"));
    REQUIRE(stopped.statusCode == 200);
    REQUIRE(stopped.body.find("\"running\":false") != std::string::npos);
}

void api_controls_software_demo_motors_as_one_scene() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    marine::MarineSafety safety(data.path() + "/events.jsonl"); marine::MarineApi api(repository, runtime, safety, motors);

    const auto reset = api.handle(request("POST", "/api/v1/marine/software-demo/reset", "{\"operator\":\"演示员\"}"));
    REQUIRE(reset.statusCode == 200);
    REQUIRE(reset.body.find("\"targetRpm\":200") != std::string::npos);
    REQUIRE(reset.body.find("\"running\":true") != std::string::npos);

    const auto execute = api.handle(request("POST", "/api/v1/marine/software-demo/execute", "{\"operator\":\"演示员\",\"motorNumbers\":[2,3,8]}"));
    REQUIRE(execute.statusCode == 200);
    REQUIRE(execute.body.find("\"controlledMotorNumbers\":[2,3]") != std::string::npos);
    REQUIRE(motors->telemetry("EXHAUST-FAN-01").running);
    REQUIRE(!motors->telemetry("FIRE-PUMP-01").running);
    REQUIRE(!motors->telemetry("DRAIN-PUMP-01").running);

    const auto stopped = api.handle(request("POST", "/api/v1/marine/software-demo/stop", "{\"operator\":\"演示员\"}"));
    REQUIRE(stopped.statusCode == 200);
    REQUIRE(!motors->telemetry("EXHAUST-FAN-01").running);
}

void api_runs_hardware_replacement_with_real_motor_state_transitions() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    marine::MarineSafety safety(data.path() + "/events.jsonl"); marine::MarineApi api(repository, runtime, safety, motors);

    const auto reset = api.handle(request("POST", "/api/v1/marine/hardware-demo/reset", "{\"operator\":\"演示员\"}"));
    REQUIRE(reset.statusCode == 200);
    REQUIRE(motors->telemetry("EXHAUST-FAN-01").running);
    REQUIRE(motors->telemetry("EXHAUST-FAN-01").targetRpm == 200);
    REQUIRE(!motors->telemetry("FIRE-PUMP-01").running);

    const auto fault = api.handle(request("POST", "/api/v1/marine/hardware-demo/fault", "{\"operator\":\"演示员\"}"));
    REQUIRE(fault.statusCode == 200);
    REQUIRE(!motors->telemetry("EXHAUST-FAN-01").running);
    REQUIRE(motors->telemetry("EXHAUST-FAN-01").actualRpm == 0);

    const auto switched = api.handle(request("POST", "/api/v1/marine/hardware-demo/switch", "{\"operator\":\"演示员\",\"deviceId\":\"MOTOR-0E\",\"speedRpm\":200,\"direction\":\"forward\"}"));
    REQUIRE(switched.statusCode == 200);
    REQUIRE(switched.body.find("\"deviceId\":\"MOTOR-0E\"") != std::string::npos);
    REQUIRE(motors->telemetry("FIRE-PUMP-01").running);
    REQUIRE(motors->telemetry("FIRE-PUMP-01").targetRpm == 200);
    const auto coolBinding = std::find_if(repository.listBindings().begin(), repository.listBindings().end(), [](const marine::Binding& value) { return value.logicalExecutorId == "COOL-01"; });
    REQUIRE(coolBinding != repository.listBindings().end());
    REQUIRE(coolBinding->deviceId == "MOTOR-0E");
}

void hardware_reset_controls_only_the_single_detected_motor() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    std::vector<uint8_t> writes;
    motors->attachIo(
        [&](uint8_t address, uint16_t, const std::vector<uint16_t>&) { writes.push_back(address); return address == 0x0E; },
        [](uint8_t address, uint16_t reg, uint16_t, std::vector<uint16_t>& values) {
            if (address != 0x0E || reg != 0x6041) return false;
            values = {0x0027}; return true;
        });
    motors->list();
    motors->list();
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    marine::MarineSafety safety(data.path() + "/events.jsonl"); marine::MarineApi api(repository, runtime, safety, motors);

    const auto reset = api.handle(request("POST", "/api/v1/marine/hardware-demo/reset", "{\"operator\":\"演示员\"}"));
    REQUIRE(reset.statusCode == 200);
    REQUIRE(!writes.empty());
    REQUIRE(std::all_of(writes.begin(), writes.end(), [](uint8_t address) { return address == 0x0E; }));
    const auto bindings = repository.listBindings();
    const auto binding = std::find_if(bindings.begin(), bindings.end(), [](const marine::Binding& value) { return value.logicalExecutorId == "COOL-01"; });
    REQUIRE(binding != bindings.end());
    REQUIRE(binding->deviceId == "MOTOR-0E");
}

void software_demo_controls_only_the_single_detected_real_motor() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    std::vector<uint8_t> writes;
    motors->attachIo(
        [&](uint8_t address, uint16_t, const std::vector<uint16_t>&) { writes.push_back(address); return address == 0x0E; },
        [](uint8_t address, uint16_t reg, uint16_t, std::vector<uint16_t>& values) {
            if (address != 0x0E) return false;
            if (reg == 0x6041) { values = {0x0027}; return true; }
            if (reg == 0x606C) { values = {0, 0}; return true; }
            return false;
        });
    motors->list();
    motors->list();
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    marine::MarineSafety safety(data.path() + "/events.jsonl"); marine::MarineApi api(repository, runtime, safety, motors);

    writes.clear();
    const auto reset = api.handle(request("POST", "/api/v1/marine/software-demo/reset", "{\"operator\":\"演示员\"}"));
    REQUIRE(reset.statusCode == 200);
    REQUIRE(reset.body.find("\"controlledMotorNumbers\":[2]") != std::string::npos);
    REQUIRE(!writes.empty());
    REQUIRE(std::all_of(writes.begin(), writes.end(), [](uint8_t address) { return address == 0x0E; }));

    writes.clear();
    const auto execute = api.handle(request("POST", "/api/v1/marine/software-demo/execute", "{\"operator\":\"演示员\",\"motorNumbers\":[1,2,5]}"));
    REQUIRE(execute.statusCode == 200);
    REQUIRE(execute.body.find("\"controlledMotorNumbers\":[2]") != std::string::npos);
    REQUIRE(!writes.empty());
    REQUIRE(std::all_of(writes.begin(), writes.end(), [](uint8_t address) { return address == 0x0E; }));

    writes.clear();
    const auto virtualOnly = api.handle(request("POST", "/api/v1/marine/software-demo/execute", "{\"operator\":\"演示员\",\"motorNumbers\":[1,5]}"));
    REQUIRE(virtualOnly.statusCode == 200);
    REQUIRE(virtualOnly.body.find("\"controlledMotorNumbers\":[]") != std::string::npos);
    REQUIRE(writes.empty());

    writes.clear();
    const auto stopped = api.handle(request("POST", "/api/v1/marine/software-demo/stop", "{\"operator\":\"演示员\"}"));
    REQUIRE(stopped.statusCode == 200);
    REQUIRE(stopped.body.find("\"controlledMotorNumbers\":[2]") != std::string::npos);
    REQUIRE(stopped.body.find("\"failedMotorNumbers\":[1,3]") != std::string::npos);
    REQUIRE(std::find(writes.begin(), writes.end(), 0x02) != writes.end());
    REQUIRE(std::find(writes.begin(), writes.end(), 0x0E) != writes.end());
    REQUIRE(std::find(writes.begin(), writes.end(), 0x0F) != writes.end());
}

void software_demo_stop_retries_transient_motor_failures() {
    TempDir data; marine::MarineRepository repository(data.path()); REQUIRE(repository.load().empty());
    auto motors = std::make_shared<marine::MotorAtomicService>(true);
    int motorOneAttempts = 0;
    motors->attachIo(
        [&](uint8_t address, uint16_t, const std::vector<uint16_t>&) {
            if (address == 0x02) return ++motorOneAttempts >= 3;
            return true;
        },
        [](uint8_t, uint16_t, uint16_t, std::vector<uint16_t>&) { return false; });
    marine::MarineRuntime runtime(repository, marine::MarineExecutor([] { return marine::SensorSnapshot::demo(); }), motors);
    marine::MarineSafety safety(data.path() + "/events.jsonl"); marine::MarineApi api(repository, runtime, safety, motors);

    const auto stopped = api.handle(request("POST", "/api/v1/marine/software-demo/stop", "{\"operator\":\"演示员\"}"));
    REQUIRE(stopped.statusCode == 200);
    REQUIRE(motorOneAttempts == 3);
    REQUIRE(stopped.body.find("\"controlledMotorNumbers\":[1,2,3]") != std::string::npos);
    REQUIRE(stopped.body.find("\"failedMotorNumbers\":[]") != std::string::npos);
}
}

int main() {
    application_validation_rejects_same_resource_in_parallel_step();
    application_json_round_trip_preserves_parallel_nodes();
    motor_parameters_round_trip_and_validate_boundaries();
    json_parser_rejects_trailing_content();
    repository_persists_application_across_reopen();
    binding_update_rejects_stale_version_without_mutation();
    repository_marks_unfinished_runs_interrupted_on_load();
    safety_defaults_to_enabled_and_latches_emergency_stop();
    executor_returns_simulated_result_without_command_callback();
    offline_sensor_is_reported_as_demo_data();
    motor_service_locks_resources_and_latches_emergency_stop();
    motor_service_reads_signed_32_bit_speed_and_reports_failed_emergency_stop();
    motor_inventory_discovers_one_device_then_heartbeats_only_that_address();
    motor_inventory_can_scan_all_addresses_without_changing_the_binding();
    inventory_refreshes_live_speed_after_startup_and_direction_changes();
    emergency_stop_preempts_a_blocked_runtime_start();
    runtime_waits_for_all_parallel_nodes_before_next_step();
    cancelling_one_run_does_not_cancel_another_run();
    api_creates_lists_and_starts_persisted_application();
    api_returns_conflict_for_stale_binding_version();
    api_runs_updates_and_emergency_stops_motor_service();
    api_directly_controls_motor_for_hardware_decoupling_screen();
    api_controls_software_demo_motors_as_one_scene();
    api_runs_hardware_replacement_with_real_motor_state_transitions();
    hardware_reset_controls_only_the_single_detected_motor();
    software_demo_controls_only_the_single_detected_real_motor();
    software_demo_stop_retries_transient_motor_failures();
    if (failures != 0) {
        std::cerr << failures << " 项测试失败\n";
        return EXIT_FAILURE;
    }
    std::cout << "marine_backend_test: 全部通过\n";
    return EXIT_SUCCESS;
}
