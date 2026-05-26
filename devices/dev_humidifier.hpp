/**
 * @file dev_humidifier.hpp
 * @brief 设备抽象层 - 恒湿净化一体机 (C++)
 */

#ifndef DEV_HUMIDIFIER_HPP
#define DEV_HUMIDIFIER_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"
#include <atomic>

class DevHumidifier : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "恒湿净化一体机"; }

    int readEnvData(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procEnvData(const uint8_t *resp, size_t resp_len, int rc);

    int readPowerState(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procPowerState(const uint8_t *resp, size_t resp_len, int rc);

    int readFaultState(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procFaultState(const uint8_t *resp, size_t resp_len, int rc);

    // 控制命令
    int setPower(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);
    int setDehumidify(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);
    int setHumidify(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);
    int setPurify(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);
    int setConstHum(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);

    int getPowerState() const { return power_state_.load(); }
    int getFaultState() const { return fault_state_.load(); }
    bool isOnline() { return status_.isOnline(); }
    EnvDataCpp getEnvData() const { return env_; }

private:
    DeviceStatusCpp status_;
    EnvDataCpp      env_;
    std::atomic<int> power_state_{0};
    std::atomic<int> fault_state_{0};
};

#endif /* DEV_HUMIDIFIER_HPP */