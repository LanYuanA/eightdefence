/**
 * @file dev_smoke.cpp
 * @brief 设备抽象层 - 烟雾报警器实现 (C++)
 */

#include "dev_smoke.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>
#include <cstring>
#include <functional>

void DevSmoke::init() {
    status_.reset();
    alarm_state_.store(0);
}

std::vector<DeviceTask> DevSmoke::getTasks() {
    std::vector<DeviceTask> tasks;
    tasks.push_back({
        "烟雾报警器 - 报警状态",
        [this](ModbusService &svc, uint8_t *resp, size_t *resp_len) -> int {
            return this->readSmoke(svc, resp, resp_len);
        },
        [this](const uint8_t *resp, size_t resp_len, int rc) {
            this->procSmoke(resp, resp_len, rc);
        },
        [](uint8_t *data, size_t cap, size_t *len) -> int {
            return ModbusService::buildReadRegFrame(DEV_SMOKE_ADDR, REG_SMOKE_ALARM, REG_SMOKE_DATA, data, cap, len);
        },
        5000, 1000, DEV_SMOKE_ADDR, 1
    });
    return tasks;
}

int DevSmoke::readSmoke(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_SMOKE_ADDR, REG_SMOKE_ALARM, REG_SMOKE_DATA,
                        resp, 512, resp_len);
}

void DevSmoke::procSmoke(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "烟雾报警器");
        return;
    }

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_SMOKE_ADDR, 0x03, 0);
    if (parse_rc != ParseService::OK) {
        logParseError(parse_rc, "烟雾报警器", "烟雾数据", resp, resp_len, DEV_SMOKE_ADDR);
        return;
    }

    status_.onSuccess();
    uint16_t data = ParseService::extractU16(resp, 0);
    alarm_state_.store((data != 0) ? 1 : 0);

    if (data != 0x0000) {
        printf("  => [🚨 烟雾报警]: 状态码 %d\n", data);
    }
}