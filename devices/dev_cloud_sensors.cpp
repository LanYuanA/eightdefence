/**
 * @file dev_cloud_sensors.cpp
 * @brief 设备抽象层 - 云测仪各传感器独立设备实现
 */

#include "dev_cloud_sensors.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>

/* ============================================================
 * PM2.5 传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudPm25::getTasks() {
    return {{
        "PM2.5传感器 - PM2.5",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudPm25::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_PM25, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudPm25::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "PM2.5传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "PM2.5传感器", "PM2.5");
    }
}

/* ============================================================
 * PM10 传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudPm10::getTasks() {
    return {{
        "PM10传感器 - PM10",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudPm10::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_PM10, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudPm10::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "PM10传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "PM10传感器", "PM10");
    }
}

/* ============================================================
 * 湿度传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudHumidity::getTasks() {
    return {{
        "湿度传感器 - 湿度",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudHumidity::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_HUMIDITY, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudHumidity::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "湿度传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "湿度传感器", "湿度");
    }
}

/* ============================================================
 * 温度传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudTemperature::getTasks() {
    return {{
        "温度传感器 - 温度",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudTemperature::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_TEMP, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudTemperature::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "温度传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        int16_t val = ParseService::extractS16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "温度传感器", "温度");
    }
}

/* ============================================================
 * TVOC 传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudTvoc::getTasks() {
    return {{
        "TVOC传感器 - TVOC",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudTvoc::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_TVOC, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudTvoc::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "TVOC传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "TVOC传感器", "TVOC");
    }
}

/* ============================================================
 * 甲醛(CH2O)传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudCh2o::getTasks() {
    return {{
        "甲醛传感器 - 甲醛",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudCh2o::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_CH2O, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudCh2o::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "甲醛传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "甲醛传感器", "甲醛");
    }
}

/* ============================================================
 * 臭氧(O3)传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudO3::getTasks() {
    return {{
        "臭氧传感器 - 臭氧",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudO3::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_O3, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudO3::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "臭氧传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "臭氧传感器", "臭氧");
    }
}

/* ============================================================
 * CO2 传感器
 * ============================================================ */
std::vector<DeviceTask> DevCloudCo2::getTasks() {
    return {{
        "CO2传感器 - CO2",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudCo2::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_CO2, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudCo2::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "CO2传感器"); return; }
    status_.onSuccess();
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r == ParseService::OK) {
        uint16_t val = ParseService::extractU16(resp, 0);
        value_.store(val);
    } else {
        logParseError(r, "CO2传感器", "CO2");
    }
}