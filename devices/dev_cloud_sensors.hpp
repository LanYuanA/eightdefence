/**
 * @file dev_cloud_sensors.hpp
 * @brief 设备抽象层 - 云测仪各传感器独立设备类 (SD123-E60V2)
 *
 * 新传感器寄存器连续排列 0x0000~0x0007:
 *   0x0000: CH2O (甲醛)    0x0001: PM2.5     0x0002: TVOC
 *   0x0003: CO2            0x0004: 温度       0x0005: 湿度
 *   0x0006: PM1.0          0x0007: PM10
 *
 * 支持两种读取模式:
 *   - 连续读 (默认): 1次Modbus读取8个寄存器, 减少总线占用
 *   - 单独读: 每个传感器独立读取1个寄存器
 *
 * 通过 g_cloud_batch_mode 全局开关控制, 运行时可切换。
 */

#ifndef DEV_CLOUD_SENSORS_HPP
#define DEV_CLOUD_SENSORS_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"
#include <atomic>

/* 连续读/单独读全局开关 (默认true=连续读) */
extern std::atomic<bool> g_cloud_batch_mode;

/* 连续读模式下的批量数据结构 */
struct CloudBatchData {
    std::atomic<uint16_t> ch2o{0};
    std::atomic<uint16_t> pm25{0};
    std::atomic<uint16_t> tvoc{0};
    std::atomic<uint16_t> co2{0};
    std::atomic<int16_t>  temperature{0};
    std::atomic<uint16_t> humidity{0};
    std::atomic<uint16_t> pm10{0};
    std::atomic<bool>     online{false};
};

/* 全局批量数据实例 (连续读模式下所有传感器共享) */
extern CloudBatchData g_cloud_batch;

/**
 * @brief 云测仪批量读取设备 (连续读模式)
 *
 * 一次 Modbus 读取 8 个连续寄存器 (0x0000~0x0007),
 * 解析后更新 g_cloud_batch 全局数据。
 */
class DevCloudBatch : public DeviceBase {
public:
    void init() override { g_cloud_batch.online.store(false); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "云测仪(批量)"; }
};

/* ============================================================
 * PM2.5 传感器
 * ============================================================ */
class DevCloudPm25 : public DeviceBase {
public:
    void init() override { status_.reset(); value_.store(0); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "PM2.5传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline();
    uint16_t getValue() const;

private:
    DeviceStatusCpp status_;
    std::atomic<uint16_t> value_{0};
};

/* ============================================================
 * PM10 传感器
 * ============================================================ */
class DevCloudPm10 : public DeviceBase {
public:
    void init() override { status_.reset(); value_.store(0); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "PM10传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline();
    uint16_t getValue() const;

private:
    DeviceStatusCpp status_;
    std::atomic<uint16_t> value_{0};
};

/* ============================================================
 * 湿度传感器
 * ============================================================ */
class DevCloudHumidity : public DeviceBase {
public:
    void init() override { status_.reset(); value_.store(0); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "湿度传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline();
    uint16_t getValue() const;

private:
    DeviceStatusCpp status_;
    std::atomic<uint16_t> value_{0};
};

/* ============================================================
 * 温度传感器
 * ============================================================ */
class DevCloudTemperature : public DeviceBase {
public:
    void init() override { status_.reset(); value_.store(0); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "温度传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline();
    int16_t getValue() const;

private:
    DeviceStatusCpp status_;
    std::atomic<int16_t> value_{0};
};

/* ============================================================
 * TVOC 传感器
 * ============================================================ */
class DevCloudTvoc : public DeviceBase {
public:
    void init() override { status_.reset(); value_.store(0); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "TVOC传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline();
    uint16_t getValue() const;

private:
    DeviceStatusCpp status_;
    std::atomic<uint16_t> value_{0};
};

/* ============================================================
 * 甲醛(CH2O)传感器
 * ============================================================ */
class DevCloudCh2o : public DeviceBase {
public:
    void init() override { status_.reset(); value_.store(0); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "甲醛传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline();
    uint16_t getValue() const;

private:
    DeviceStatusCpp status_;
    std::atomic<uint16_t> value_{0};
};

/* ============================================================
 * CO2 传感器
 * ============================================================ */
class DevCloudCo2 : public DeviceBase {
public:
    void init() override { status_.reset(); value_.store(0); }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "CO2传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline();
    uint16_t getValue() const;

private:
    DeviceStatusCpp status_;
    std::atomic<uint16_t> value_{0};
};

#endif /* DEV_CLOUD_SENSORS_HPP */