/**
 * @file dev_air_purifier.cpp
 * @brief 设备抽象层 - 霉菌空气净化机实现 (C++) — 执行器 + 可选读取
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
    // 仅轮询电源状态，环境数据和运行模式不轮询
    std::vector<DeviceTask> tasks;
    tasks.push_back({
        "霉菌空气净化机 - 电源状态",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readPowerState(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procPowerState(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_PURIFIER_ADDR, REG_PUR_POWER_STATUS, 1, d, c, l); },
        5000, 1000, DEV_PURIFIER_ADDR
    });
    return tasks;
}

// --- 读取指令（封装但暂不轮询） ---

int DevAirPurifier::readEnvData(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_PURIFIER_ADDR, REG_PUR_CO2, REG_PUR_READ_COUNT, resp, 512, resp_len);
}

void DevAirPurifier::procEnvData(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "霉菌空气净化机"); return; }
    status_.onSuccess();
    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 0);
    if (parse_rc == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        std::lock_guard<std::mutex> lock(env_.mtx);
        env_.co2 = val;
    }
}

int DevAirPurifier::readRunMode(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_PURIFIER_ADDR, REG_PUR_RUN_MODE, 1, resp, 512, resp_len);
}

void DevAirPurifier::procRunMode(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "霉菌空气净化机"); return; }
    status_.onSuccess();
    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 0);
    if (parse_rc == ParseService::OK) {
        run_mode_.store(ParseService::extractU16(resp, 0));
    }
}

int DevAirPurifier::readPowerState(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_PURIFIER_ADDR, REG_PUR_POWER_STATUS, 1, resp, 512, resp_len);
}

void DevAirPurifier::procPowerState(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "霉菌空气净化机"); return; }
    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 0);
    if (parse_rc != ParseService::OK) { logParseError(parse_rc, "霉菌空气净化机", "电源状态", resp, resp_len, DEV_PURIFIER_ADDR); return; }
    status_.onSuccess();
    power_status_.store(ParseService::extractU16(resp, 0));
}

// --- 控制命令 ---

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
