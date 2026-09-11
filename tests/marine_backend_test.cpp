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
    const auto stopped = api.handle(request("POST", "/api/v1/marine/emergency-stop", "{\"operator\":\"演示员\"}")); REQUIRE(stopped.statusCode == 200); REQUIRE(stopped.body.find("true") != std::string::npos);
    const auto reset = api.handle(request("POST", "/api/v1/marine/emergency-reset", "{\"operator\":\"演示员\"}")); REQUIRE(reset.statusCode == 200);
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
    runtime_waits_for_all_parallel_nodes_before_next_step();
    cancelling_one_run_does_not_cancel_another_run();
    api_creates_lists_and_starts_persisted_application();
    api_returns_conflict_for_stale_binding_version();
    api_runs_updates_and_emergency_stops_motor_service();
    if (failures != 0) {
        std::cerr << failures << " 项测试失败\n";
        return EXIT_FAILURE;
    }
    std::cout << "marine_backend_test: 全部通过\n";
    return EXIT_SUCCESS;
}
