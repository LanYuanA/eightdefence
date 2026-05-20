/**
 * @file dev_cloud_sensors.hpp
 * @brief 设备抽象层 - 云测仪各传感器独立设备类
 *
 * 将原 DevCloud 拆分为独立的传感器设备抽象:
 *   DevCloudPm25, DevCloudPm10, DevCloudHumidity, DevCloudTemperature,
 *   DevCloudTvoc, DevCloudCh2o, DevCloudO3, DevCloudCo2
 *
 * 所有传感器共享同一物理设备地址 DEV_CLOUD_ADDR (0x01),
 * 但各自独立管理状态和任务。
 */

#ifndef DEV_CLOUD_SENSORS_HPP
#define DEV_CLOUD_SENSORS_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"

/* ============================================================
 * PM2.5 传感器
 * ============================================================ */
class DevCloudPm25 : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "PM2.5传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    uint16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    uint16_t value_ = 0;
};

/* ============================================================
 * PM10 传感器
 * ============================================================ */
class DevCloudPm10 : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "PM10传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    uint16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    uint16_t value_ = 0;
};

/* ============================================================
 * 湿度传感器
 * ============================================================ */
class DevCloudHumidity : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "湿度传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    uint16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    uint16_t value_ = 0;
};

/* ============================================================
 * 温度传感器
 * ============================================================ */
class DevCloudTemperature : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "温度传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    int16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    int16_t value_ = 0;
};

/* ============================================================
 * TVOC 传感器
 * ============================================================ */
class DevCloudTvoc : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "TVOC传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    uint16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    uint16_t value_ = 0;
};

/* ============================================================
 * 甲醛(CH2O)传感器
 * ============================================================ */
class DevCloudCh2o : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "甲醛传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    uint16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    uint16_t value_ = 0;
};

/* ============================================================
 * 臭氧(O3)传感器
 * ============================================================ */
class DevCloudO3 : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "臭氧传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    uint16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    uint16_t value_ = 0;
};

/* ============================================================
 * CO2 传感器
 * ============================================================ */
class DevCloudCo2 : public DeviceBase {
public:
    void init() override { status_.reset(); value_ = 0; }
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "CO2传感器"; }

    int readValue(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procValue(const uint8_t *resp, size_t resp_len, int rc);
    bool isOnline() { return status_.isOnline(); }
    uint16_t getValue() const { return value_; }

private:
    DeviceStatusCpp status_;
    uint16_t value_ = 0;
};

#endif /* DEV_CLOUD_SENSORS_HPP */