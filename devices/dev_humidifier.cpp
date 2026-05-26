/**
 * @file dev_humidifier.cpp
 * @brief 设备抽象层 - 恒湿净化一体机实现 (C++)
 */

#include "dev_humidifier.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>
#include <functional>

void DevHumidifier::init() {
    status_.reset();
    env_ = EnvDataCpp{};
    power_state_.store(0);
    fault_state_.store(0);
}

std::vector<DeviceTask> DevHumidifier::getTasks() {
    std::vector<DeviceTask> tasks;

    tasks.push_back({
        "恒湿净化一体机 - 环境数据",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readEnvData(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procEnvData(r, l, rc); },
        5000, 1000, DEV_HUMIDIFIER_ADDR
    });
    tasks.push_back({
        "恒湿净化一体机 - 电源状态",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readPowerState(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procPowerState(r, l, rc); },
        5000, 1000, DEV_HUMIDIFIER_ADDR
    });
    tasks.push_back({
        "恒湿净化一体机 - 故障状态",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readFaultState(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procFaultState(r, l, rc); },
        5000, 1000, DEV_HUMIDIFIER_ADDR
    });

    return tasks;
}

int DevHumidifier::readEnvData(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_HUMIDIFIER_ADDR, REG_HUM_CO2, REG_HUM_READ_COUNT,
                        resp, 512, resp_len);
}

int DevHumidifier::readPowerState(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_HUMIDIFIER_ADDR, REG_HUM_POWER_STATE, 1,
                        resp, 512, resp_len);
}

int DevHumidifier::readFaultState(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_HUMIDIFIER_ADDR, REG_HUM_FAULT_STATE, 1,
                        resp, 512, resp_len);
}

void DevHumidifier::procEnvData(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "恒湿净化一体机"); return; }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_HUMIDIFIER_ADDR, 0x03, 10);
    if (parse_rc == ParseService::OK) {
        int co2  = ParseService::extractU16(resp, 0);
        int ch2o = ParseService::extractU16(resp, 1);
        int tvoc = ParseService::extractU16(resp, 2);
        int pm25 = ParseService::extractU16(resp, 3);
        int pm10 = ParseService::extractU16(resp, 4);
        {
            std::lock_guard<std::mutex> lock(env_.mtx);
            env_.co2 = co2; env_.ch2o = ch2o;
            env_.tvoc = tvoc; env_.pm25 = pm25; env_.pm10 = pm10;
        }
        printf("  => [📊 恒湿机环境]: CO2=%d, 甲醛=%d, TVOC=%d, PM2.5=%d, PM10=%d\n",
               co2, ch2o, tvoc, pm25, pm10);
    } else {
        printf("  => [❌ 解析失败]: 恒湿机环境数据 响应格式不符合预期协议\n");
    }
}

void DevHumidifier::procPowerState(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "恒湿净化一体机"); return; }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_HUMIDIFIER_ADDR, 0x03, 2);
    if (parse_rc == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        power_state_.store(val);
        printf("  => [⚡ 恒湿机电源]: %s\n", val ? "开机" : "关机");
    }
}

void DevHumidifier::procFaultState(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "恒湿净化一体机"); return; }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_HUMIDIFIER_ADDR, 0x03, 2);
    if (parse_rc == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        fault_state_.store(val);
        if (val == 0) {
            printf("  => [✅ 恒湿机故障]: 正常，无故障\n");
        } else {
            printf("  => [🚨 恒湿机故障]: 故障码 0x%04X\n", val);
        }
    }
}

int DevHumidifier::setPower(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_POWER, val, resp, 512, resp_len);
}

int DevHumidifier::setDehumidify(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_DEHUMIDIFY, val, resp, 512, resp_len);
}

int DevHumidifier::setHumidify(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_HUMIDIFY, val, resp, 512, resp_len);
}

int DevHumidifier::setPurify(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_PURIFY, val, resp, 512, resp_len);
}

int DevHumidifier::setConstHum(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_CONST_HUM, val, resp, 512, resp_len);
}