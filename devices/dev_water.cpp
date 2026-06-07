/**
 * @file dev_water.cpp
 * @brief 设备抽象层 - 水浸传感器实现 (C++)
 */

#include "dev_water.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>
#include <functional>

void DevWater::init() {
    status_.reset();
    water_state_.store(0);
}

std::vector<DeviceTask> DevWater::getTasks() {
    std::vector<DeviceTask> tasks;
    tasks.push_back({
        "水浸传感器 - 浸水状态",
        [this](ModbusService &svc, uint8_t *resp, size_t *resp_len) -> int {
            return this->readWater(svc, resp, resp_len);
        },
        [this](const uint8_t *resp, size_t resp_len, int rc) {
            this->procWater(resp, resp_len, rc);
        },
        [](uint8_t *data, size_t cap, size_t *len) -> int {
            return ModbusService::buildReadRegFrame(DEV_WATER_ADDR, REG_WATER_STATUS, REG_WATER_DATA, data, cap, len);
        },
        5000, 1000, DEV_WATER_ADDR, 1
    });
    return tasks;
}

int DevWater::readWater(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_WATER_ADDR, REG_WATER_STATUS, REG_WATER_DATA,
                        resp, 512, resp_len);
}

void DevWater::procWater(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "水浸传感器");
        return;
    }

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_WATER_ADDR, 0x03, 0);
    if (parse_rc != ParseService::OK) {
        logParseError(parse_rc, "水浸传感器", "水浸数据", resp, resp_len, DEV_WATER_ADDR);
        return;
    }

    status_.onSuccess();
    uint16_t data = ParseService::extractU16(resp, 0);
    water_state_.store((data != 0) ? 1 : 0);

    if (data != 0x0000) {
        printf("  => [🚨 水浸报警]: 状态码 %d\n", data);
    }
}