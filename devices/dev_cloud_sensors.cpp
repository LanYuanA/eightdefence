/**
 * @file dev_cloud_sensors.cpp
 * @brief 设备抽象层 - 云测仪传感器实现 (SD123-E60V2)
 *
 * 支持两种读取模式:
 *   连续读 (g_cloud_batch_mode=true): DevCloudBatch 一次读8个寄存器
 *   单独读 (g_cloud_batch_mode=false): 每个传感器独立读取
 */

#include "dev_cloud_sensors.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"
#include <cstdio>

/* ============================================================
 * 全局开关和批量数据
 * ============================================================ */
std::atomic<bool> g_cloud_batch_mode{true};  /* 默认连续读 */
CloudBatchData g_cloud_batch;

/* ============================================================
 * 云测仪批量读取 (连续读模式)
 * 一次读 0x0000~0x0007 共 8 个寄存器
 * ============================================================ */
std::vector<DeviceTask> DevCloudBatch::getTasks() {
    return {{
        "云测仪 - 批量读取(CH2O/PM2.5/TVOC/CO2/Temp/Humi/PM10)",
        [](ModbusService &svc, uint8_t *resp, size_t *resp_len) -> int {
            return svc.readReg(DEV_CLOUD_ADDR, 0x0000, 8, resp, 512, resp_len);
        },
        [](const uint8_t *resp, size_t resp_len, int rc) {
            if (rc != 0) {
                g_cloud_batch.online.store(false);
                return;
            }
            int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
            if (r != ParseService::OK) {
                g_cloud_batch.online.store(false);
                return;
            }
            /* 解析 8 个寄存器 (extractU16 参数是字索引, 不是字节偏移) */
            g_cloud_batch.ch2o.store(ParseService::extractU16(resp, 0));       /* word_idx=0 → 0x0000 甲醛 ppb */
            g_cloud_batch.pm25.store(ParseService::extractU16(resp, 1));      /* word_idx=1 → 0x0001 PM2.5 */
            g_cloud_batch.tvoc.store(ParseService::extractU16(resp, 2));      /* word_idx=2 → 0x0002 TVOC */
            g_cloud_batch.co2.store(ParseService::extractU16(resp, 3));       /* word_idx=3 → 0x0003 CO2 */
            g_cloud_batch.temperature.store(ParseService::extractS16(resp, 4)); /* word_idx=4 → 0x0004 温度 (有符号) */
            g_cloud_batch.humidity.store(ParseService::extractU16(resp, 5));  /* word_idx=5 → 0x0005 湿度 */
            /* word_idx=6 → 0x0006 PM1.0 跳过 */
            g_cloud_batch.pm10.store(ParseService::extractU16(resp, 7));     /* word_idx=7 → 0x0007 PM10 */
            g_cloud_batch.online.store(true);
        },
        [](uint8_t *d, size_t c, size_t *l) -> int {
            return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, 0x0000, 8, d, c, l);
        },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

/* ============================================================
 * 各传感器 isOnline/getValue: 连续读模式下从批量数据读取
 * ============================================================ */
bool DevCloudPm25::isOnline() {
    return g_cloud_batch_mode.load() ? g_cloud_batch.online.load() : status_.isOnline();
}
uint16_t DevCloudPm25::getValue() const {
    return g_cloud_batch_mode.load() ? g_cloud_batch.pm25.load() : value_.load();
}

bool DevCloudPm10::isOnline() {
    return g_cloud_batch_mode.load() ? g_cloud_batch.online.load() : status_.isOnline();
}
uint16_t DevCloudPm10::getValue() const {
    return g_cloud_batch_mode.load() ? g_cloud_batch.pm10.load() : value_.load();
}

bool DevCloudHumidity::isOnline() {
    return g_cloud_batch_mode.load() ? g_cloud_batch.online.load() : status_.isOnline();
}
uint16_t DevCloudHumidity::getValue() const {
    return g_cloud_batch_mode.load() ? g_cloud_batch.humidity.load() : value_.load();
}

bool DevCloudTemperature::isOnline() {
    return g_cloud_batch_mode.load() ? g_cloud_batch.online.load() : status_.isOnline();
}
int16_t DevCloudTemperature::getValue() const {
    return g_cloud_batch_mode.load() ? g_cloud_batch.temperature.load() : value_.load();
}

bool DevCloudTvoc::isOnline() {
    return g_cloud_batch_mode.load() ? g_cloud_batch.online.load() : status_.isOnline();
}
uint16_t DevCloudTvoc::getValue() const {
    return g_cloud_batch_mode.load() ? g_cloud_batch.tvoc.load() : value_.load();
}

bool DevCloudCh2o::isOnline() {
    return g_cloud_batch_mode.load() ? g_cloud_batch.online.load() : status_.isOnline();
}
uint16_t DevCloudCh2o::getValue() const {
    return g_cloud_batch_mode.load() ? g_cloud_batch.ch2o.load() : value_.load();
}

bool DevCloudCo2::isOnline() {
    return g_cloud_batch_mode.load() ? g_cloud_batch.online.load() : status_.isOnline();
}
uint16_t DevCloudCo2::getValue() const {
    return g_cloud_batch_mode.load() ? g_cloud_batch.co2.load() : value_.load();
}

/* ============================================================
 * 各传感器 getTasks: 连续读模式下返回空 (由 DevCloudBatch 统一读取)
 *                    单独读模式下返回各自的读取任务
 * ============================================================ */
std::vector<DeviceTask> DevCloudPm25::getTasks() {
    if (g_cloud_batch_mode.load()) return {};  /* 连续读模式: 不产生独立任务 */
    return {{
        "PM2.5传感器 - PM2.5",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, REG_CLOUD_PM25, REG_CLOUD_DATA, d, c, l); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudPm25::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_PM25, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudPm25::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "PM2.5传感器"); return; }
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r != ParseService::OK) { logParseError(r, "PM2.5传感器", "PM2.5", resp, resp_len, DEV_CLOUD_ADDR); return; }
    status_.onSuccess();
    value_.store(ParseService::extractU16(resp, 0));
}

std::vector<DeviceTask> DevCloudPm10::getTasks() {
    if (g_cloud_batch_mode.load()) return {};
    return {{
        "PM10传感器 - PM10",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, REG_CLOUD_PM10, REG_CLOUD_DATA, d, c, l); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudPm10::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_PM10, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudPm10::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "PM10传感器"); return; }
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r != ParseService::OK) { logParseError(r, "PM10传感器", "PM10", resp, resp_len, DEV_CLOUD_ADDR); return; }
    status_.onSuccess();
    value_.store(ParseService::extractU16(resp, 0));
}

std::vector<DeviceTask> DevCloudHumidity::getTasks() {
    if (g_cloud_batch_mode.load()) return {};
    return {{
        "湿度传感器 - 湿度",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, REG_CLOUD_HUMIDITY, REG_CLOUD_DATA, d, c, l); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudHumidity::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_HUMIDITY, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudHumidity::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "湿度传感器"); return; }
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r != ParseService::OK) { logParseError(r, "湿度传感器", "湿度", resp, resp_len, DEV_CLOUD_ADDR); return; }
    status_.onSuccess();
    value_.store(ParseService::extractU16(resp, 0));
}

std::vector<DeviceTask> DevCloudTemperature::getTasks() {
    if (g_cloud_batch_mode.load()) return {};
    return {{
        "温度传感器 - 温度",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, REG_CLOUD_TEMP, REG_CLOUD_DATA, d, c, l); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudTemperature::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_TEMP, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudTemperature::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "温度传感器"); return; }
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r != ParseService::OK) { logParseError(r, "温度传感器", "温度", resp, resp_len, DEV_CLOUD_ADDR); return; }
    status_.onSuccess();
    value_.store(ParseService::extractS16(resp, 0));
}

std::vector<DeviceTask> DevCloudTvoc::getTasks() {
    if (g_cloud_batch_mode.load()) return {};
    return {{
        "TVOC传感器 - TVOC",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, REG_CLOUD_TVOC, REG_CLOUD_DATA, d, c, l); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudTvoc::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_TVOC, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudTvoc::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "TVOC传感器"); return; }
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r != ParseService::OK) { logParseError(r, "TVOC传感器", "TVOC", resp, resp_len, DEV_CLOUD_ADDR); return; }
    status_.onSuccess();
    value_.store(ParseService::extractU16(resp, 0));
}

std::vector<DeviceTask> DevCloudCh2o::getTasks() {
    if (g_cloud_batch_mode.load()) return {};
    return {{
        "甲醛传感器 - 甲醛",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, REG_CLOUD_CH2O, REG_CLOUD_DATA, d, c, l); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudCh2o::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_CH2O, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudCh2o::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "甲醛传感器"); return; }
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r != ParseService::OK) { logParseError(r, "甲醛传感器", "甲醛", resp, resp_len, DEV_CLOUD_ADDR); return; }
    status_.onSuccess();
    value_.store(ParseService::extractU16(resp, 0));
}

std::vector<DeviceTask> DevCloudCo2::getTasks() {
    if (g_cloud_batch_mode.load()) return {};
    return {{
        "CO2传感器 - CO2",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readValue(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procValue(r, l, rc); },
        [](uint8_t *d, size_t c, size_t *l) -> int { return ModbusService::buildReadRegFrame(DEV_CLOUD_ADDR, REG_CLOUD_CO2, REG_CLOUD_DATA, d, c, l); },
        5000, 1000, DEV_CLOUD_ADDR
    }};
}

int DevCloudCo2::readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_CLOUD_ADDR, REG_CLOUD_CO2, REG_CLOUD_DATA, resp, 512, resp_len);
}

void DevCloudCo2::procValue(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handleFailure(status_, "CO2传感器"); return; }
    int r = ParseService::parseDeviceData(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 0);
    if (r != ParseService::OK) { logParseError(r, "CO2传感器", "CO2", resp, resp_len, DEV_CLOUD_ADDR); return; }
    status_.onSuccess();
    value_.store(ParseService::extractU16(resp, 0));
}
