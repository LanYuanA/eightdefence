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
    infrared_state_ = 0;
    radar_state_ = 0;
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
        }
    });

    tasks.push_back({
        "红外探测器 - 雷达状态",
        [this](ModbusService &svc, uint8_t *resp, size_t *resp_len) -> int {
            return this->readRadar(svc, resp, resp_len);
        },
        [this](const uint8_t *resp, size_t resp_len, int rc) {
            this->procRadar(resp, resp_len, rc);
        }
    });

    return tasks;
}

int DevInfrared::readInfrared(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readCoil(DEV_INFRARED_ADDR, REG_INFRARED_STATE, REG_INFRARED_DATA,
                         resp, 512, resp_len);
}

void DevInfrared::procInfrared(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "红外探测器");
        return;
    }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_INFRARED_ADDR, 0x01, 0);
    if (parse_rc == ParseService::OK) {
        uint16_t data = static_cast<uint16_t>(ParseService::extractBool(resp, 0));
        infrared_state_ = (data != 0) ? 1 : 0;

        if (data == 0x0000) {
            printf("  => [✅ 红外状态]: 正常，未检测到人体\n");
        } else if (data == 0x0001) {
            printf("  => [🚨 红外报警]: 检测到有人体入侵！\n");
        } else {
            printf("  => [⚠️ 未知状态]: 收到非标准红外状态码: %d\n", data);
        }
    } else {
        logParseError(parse_rc, "红外探测器", "红外数据");
    }
}

int DevInfrared::readRadar(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readCoil(DEV_INFRARED_ADDR, REG_RADAR_STATE, REG_INFRARED_DATA,
                         resp, 512, resp_len);
}

void DevInfrared::procRadar(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        handleFailure(status_, "红外探测器");
        return;
    }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_INFRARED_ADDR, 0x01, 0);
    if (parse_rc == ParseService::OK) {
        uint16_t data = static_cast<uint16_t>(ParseService::extractBool(resp, 0));
        radar_state_ = (data != 0) ? 1 : 0;

        if (data == 0x0000) {
            printf("  => [✅ 雷达状态]: 正常，未检测到移动\n");
        } else if (data == 0x0001) {
            printf("  => [🚨 雷达报警]: 检测到有移动入侵！\n");
        } else {
            printf("  => [⚠️ 未知状态]: 收到非标准雷达状态码: %d\n", data);
        }
    } else {
        logParseError(parse_rc, "红外探测器", "雷达数据");
    }
}