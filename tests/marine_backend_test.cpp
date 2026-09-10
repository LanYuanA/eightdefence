#include "application/marine/json_value.hpp"
#include "application/marine/marine_types.hpp"

#include <cstdlib>
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
}

int main() {
    application_validation_rejects_same_resource_in_parallel_step();
    application_json_round_trip_preserves_parallel_nodes();
    json_parser_rejects_trailing_content();
    if (failures != 0) {
        std::cerr << failures << " 项测试失败\n";
        return EXIT_FAILURE;
    }
    std::cout << "marine_backend_test: 全部通过\n";
    return EXIT_SUCCESS;
}
