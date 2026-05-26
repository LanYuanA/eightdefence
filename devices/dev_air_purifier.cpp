/**
 * @file dev_air_purifier.cpp
 * @brief 设备抽象层 - 霉菌空气净化机实现 (C++)
 */

#include "dev_air_purifier.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>

void DevAirPurifier::init() {
    status_.reset();
    env_ = EnvDataCpp{};
    run_mode_.store(0);
    power_status_.store(0);
}

std::vector<DeviceTask> DevAirPurifier::getTasks() {
    std::vector<DeviceTask> tasks;

    tasks.push_back({
        "霉菌空气净化机 - 环境数据",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readEnvData(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procEnvData(r, l, rc); },
        5000, 1000, DEV_PURIFIER_ADDR
    });
    tasks.push_back({
        "霉菌空气净化机 - 运行模式",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readRunMode(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procRunMode(r, l, rc); },
        5000, 1000, DEV_PURIFIER_ADDR
    });
    tasks.push_back({
        "霉菌空气净化机 - 电源状态",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readPowerState(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procPowerState(r, l, rc); },
        5000, 1000, DEV_PURIFIER_ADDR
    });

    return tasks;
}

int DevAirPurifier::readEnvData(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_PURIFIER_ADDR, REG_PUR_CO2, REG_PUR_READ_COUNT,
                        resp, 512, resp_len);
}

int DevAirPurifier::readRunMode(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_PURIFIER_ADDR, REG_PUR_RUN_MODE, 1,
                        resp, 512, resp_len);
}

int DevAirPurifier::readPowerState(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_PURIFIER_ADDR, REG_PUR_POWER_STATUS, 1,
                        resp, 512, resp_len);
}

void DevAirPurifier::procEnvData(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "霉菌空气净化机"); return; }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 2);
    if (parse_rc == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        {
            std::lock_guard<std::mutex> lock(env_.mtx);
            env_.co2 = val;
        }
        printf("  => [📊 净化机环境]: CO2=%d\n", val);
    } else {
        printf("  => [❌ 解析失败]: 净化机环境数据 响应格式不符合预期协议\n");
    }
}

void DevAirPurifier::procRunMode(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "霉菌空气净化机"); return; }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 2);
    if (parse_rc == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        run_mode_.store(val);
        printf("  => [🔄 净化机模式]: 运行模式=%d\n", val);
    }
}

void DevAirPurifier::procPowerState(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "霉菌空气净化机"); return; }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 2);
    if (parse_rc == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        power_status_.store(val);
        printf("  => [⚡ 净化机电源]: %s\n", val ? "开机" : "关机");
    }
}

int DevAirPurifier::setPower(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_PURIFIER_ADDR, REG_PUR_CTRL_POWER, val, resp, 512, resp_len);
}

int DevAirPurifier::setRunMode(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_PURIFIER_ADDR, REG_PUR_CTRL_RUN_MODE, val, resp, 512, resp_len);
}

int DevAirPurifier::setManual(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_PURIFIER_ADDR, REG_PUR_CTRL_MANUAL, val, resp, 512, resp_len);
}

int DevAirPurifier::setTiming(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_PURIFIER_ADDR, REG_PUR_CTRL_TIMING, val, resp, 512, resp_len);
}