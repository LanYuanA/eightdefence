/**
 * @file dev_infrared.cpp
 * @brief 设备抽象层 - 红外探测器实现 (C++)
 */

#include "dev_infrared.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>
#include <functional>

void DevInfrared::init() {
    status_.reset();
    infrared_state_.store(0);
    radar_state_.store(0);
}

std::vector<DeviceTask> DevInfrared::getTasks() {
    std::vector<DeviceTask> tasks;

    tasks.push_back({
        "红外探测器 - 红外状态",
        [this](ModbusService &svc, uint8_t *resp, size_t *resp_len) -> int {
            return this->readInfrared(svc, resp, resp_len);
        },
        [this](const uint8_t *resp, size_t resp_len, int rc) {
            this->procInfrared(resp, resp_len, rc);
        },
        5000, 1000, DEV_INFRARED_ADDR, 1
    });

    tasks.push_back({
        "红外探测器 - 雷达状态",
        [this](ModbusService &svc, uint8_t *resp, size_t *resp_len) -> int {
            return this->readRadar(svc, resp, resp_len);
        },
        [this](const uint8_t *resp, size_t resp_len, int rc) {
            this->procRadar(resp, resp_len, rc);
        },
        5000, 1000, DEV_INFRARED_ADDR, 1
    });

    return tasks;
}

int DevInfrared::readInfrared(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_INFRARED_ADDR, REG_INFRARED_STATE, REG_INFRARED_DATA,
                        resp, 512, resp_len);
}

void DevInfrared::procInfrared(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "红外探测器");
        return;
    }

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_INFRARED_ADDR, 0x03, 0);
    if (parse_rc != ParseService::OK) {
        logParseError(parse_rc, "红外探测器", "红外数据", resp, resp_len, DEV_INFRARED_ADDR);
        return;
    }

    status_.onSuccess();
    uint16_t data = ParseService::extractU16(resp, 0);
    infrared_state_.store((data != 0) ? 1 : 0);

    if (data != 0x0000) {
        printf("  => [🚨 红外报警]: 状态码 %d\n", data);
    }
}

int DevInfrared::readRadar(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_INFRARED_ADDR, REG_RADAR_STATE, REG_INFRARED_DATA,
                        resp, 512, resp_len);
}

void DevInfrared::procRadar(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "红外探测器");
        return;
    }

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_INFRARED_ADDR, 0x03, 0);
    if (parse_rc != ParseService::OK) {
        logParseError(parse_rc, "红外探测器", "雷达数据", resp, resp_len, DEV_INFRARED_ADDR);
        return;
    }

    status_.onSuccess();
    uint16_t data = ParseService::extractU16(resp, 0);
    radar_state_.store((data != 0) ? 1 : 0);

    if (data != 0x0000) {
        printf("  => [🚨 雷达报警]: 状态码 %d\n", data);
    }
}