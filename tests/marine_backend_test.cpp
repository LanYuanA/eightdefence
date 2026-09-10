#include "application/marine/json_value.hpp"
#include "application/marine/marine_types.hpp"
#include "application/marine/marine_repository.hpp"
#include "application/marine/marine_safety.hpp"
#include "application/marine/marine_executor.hpp"
#include "application/marine/marine_runtime.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

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
    const auto input = makeApplication({{"A01"}, {"02", "03"}});
    marine::JsonValue value;
    std::string error;
    REQUIRE(marine::parseJson(marine::toJson(marine::applicationToJson(input)), value, error));
    marine::Application output;
    REQUIRE(marine::applicationFromJson(value, output, error));
    REQUIRE(output.steps.size() == 2);
    REQUIRE(output.steps.at(1).nodes.size() == 2);
    REQUIRE(output.steps.at(1).nodes.at(1).serviceId == "03");
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

void safety_unlock_expires_after_ten_minutes() {
    TempDir data;
    marine::MarineSafety safety(data.path() + "/events.jsonl");
    safety.unlock("演示员", 1000);
    REQUIRE(safety.status(600999).unlocked);
    REQUIRE(!safety.status(601000).unlocked);
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
}

int main() {
    application_validation_rejects_same_resource_in_parallel_step();
    application_json_round_trip_preserves_parallel_nodes();
    json_parser_rejects_trailing_content();
    repository_persists_application_across_reopen();
    binding_update_rejects_stale_version_without_mutation();
    repository_marks_unfinished_runs_interrupted_on_load();
    safety_unlock_expires_after_ten_minutes();
    executor_returns_simulated_result_without_command_callback();
    offline_sensor_is_reported_as_demo_data();
    runtime_waits_for_all_parallel_nodes_before_next_step();
    cancelling_one_run_does_not_cancel_another_run();
    if (failures != 0) {
        std::cerr << failures << " 项测试失败\n";
        return EXIT_FAILURE;
    }
    std::cout << "marine_backend_test: 全部通过\n";
    return EXIT_SUCCESS;
}
