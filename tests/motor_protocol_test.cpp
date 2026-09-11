#include "devices/dev_stepper_motor.hpp"
#include "service/modbus_service.hpp"
#include "core/async_bus.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <queue>

namespace {
int failures = 0;
#define REQUIRE(x) do { if (!(x)) { std::cerr << "失败: " #x "\n"; ++failures; } } while (false)
}

int main() {
    const auto forward = motorI32Words(200);
    REQUIRE(forward[0] == 0x0000);
    REQUIRE(forward[1] == 0x00C8);
    const auto reverse = motorI32Words(-200);
    REQUIRE(reverse[0] == 0xFFFF);
    REQUIRE(reverse[1] == 0xFF38);

    uint8_t frame[32]{};
    size_t length = 0;
    const uint16_t values[2] = {reverse[0], reverse[1]};
    REQUIRE(ModbusService::buildWriteMultiRegFrame(0x0E, 0x60FF, 2, values, frame, sizeof(frame), &length) == 0);
    REQUIRE(length == 13);
    REQUIRE(frame[0] == 0x0E && frame[1] == 0x10);
    REQUIRE(frame[7] == 0xFF && frame[8] == 0xFF && frame[9] == 0xFF && frame[10] == 0x38);

    std::priority_queue<AsyncRequest> requests;
    AsyncRequest normalWrite; normalWrite.isWrite = true; normalWrite.priority = 1; normalWrite.sequence = 1;
    AsyncRequest urgentStop; urgentStop.isWrite = true; urgentStop.priority = 3; urgentStop.sequence = 3;
    AsyncRequest earlierUrgent; earlierUrgent.isWrite = true; earlierUrgent.priority = 3; earlierUrgent.sequence = 2;
    requests.push(normalWrite); requests.push(urgentStop); requests.push(earlierUrgent);
    REQUIRE(requests.top().priority == 3 && requests.top().sequence == 2);
    requests.pop();
    REQUIRE(requests.top().priority == 3 && requests.top().sequence == 3);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
