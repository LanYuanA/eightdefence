/**
 * @file dev_light.cpp
 * @brief 设备抽象层 - 弱光检测传感器实现 (C++)
 */

#include "dev_light.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>
#include <functional>

void DevLight::init() {
    status_.reset();
    illuminance_.store(0);
}

std::vector<DeviceTask> DevLight::getTasks() {
    std::vector<DeviceTask> tasks;
    tasks.push_back({
        "弱光检测传感器 - 光照度",
        [this](ModbusService &svc, uint8_t *resp, size_t *resp_len) -> int {
            return this->readLight(svc, resp, resp_len);
        },
        [this](const uint8_t *resp, size_t resp_len, int rc) {
            this->procLight(resp, resp_len, rc);
        },
        5000, 1000, DEV_LIGHT_ADDR
    });
    return tasks;
}

int DevLight::readLight(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_LIGHT_ADDR, REG_LIGHT_ILLUMIN, REG_LIGHT_DATA,
                        resp, 512, resp_len);
}

void DevLight::procLight(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "弱光检测传感器");
        return;
    }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_LIGHT_ADDR, 0x03, 0);
    if (parse_rc == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        illuminance_.store(val);
    } else {
        logParseError(parse_rc, "弱光检测传感器", "光照数据");
    }
}