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
    water_state_ = 0;
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
        }
    });
    return tasks;
}

int DevWater::readWater(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readCoil(DEV_WATER_ADDR, REG_WATER_STATUS, REG_WATER_DATA,
                         resp, 512, resp_len);
}

void DevWater::procWater(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "水浸传感器");
        return;
    }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_WATER_ADDR, 0x01, 0);
    if (parse_rc == ParseService::OK) {
        uint16_t data = static_cast<uint16_t>(ParseService::extractBool(resp, 0));
        water_state_ = (data != 0) ? 1 : 0;

        if (data == 0x0000) {
            printf("  => [✅ 水浸状态]: 正常，未检测到水浸\n");
        } else if (data == 0x0001) {
            printf("  => [🚨 水浸报警]: 检测到有水浸入侵！\n");
        } else {
            printf("  => [⚠️ 未知状态]: 收到非标准水浸状态码: %d\n", data);
        }
    } else {
        logParseError(parse_rc, "水浸传感器", "水浸数据");
    }
}