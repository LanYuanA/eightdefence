/**
 * @file dev_air_purifier.hpp
 * @brief 设备抽象层 - 霉菌空气净化机 (C++)
 */

#ifndef DEV_AIR_PURIFIER_HPP
#define DEV_AIR_PURIFIER_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"

class DevAirPurifier : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "霉菌空气净化机"; }

    int readEnvData(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procEnvData(const uint8_t *resp, size_t resp_len, int rc);

    int readRunMode(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procRunMode(const uint8_t *resp, size_t resp_len, int rc);

    int readPowerState(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procPowerState(const uint8_t *resp, size_t resp_len, int rc);

    // 控制命令
    int setPower(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);
    int setRunMode(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);
    int setManual(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);
    int setTiming(ModbusService &svc, uint16_t val, uint8_t *resp, size_t *resp_len);

    int getRunMode() const { return run_mode_; }
    int getPowerStatus() const { return power_status_; }
    bool isOnline() { return status_.isOnline(); }

private:
    DeviceStatusCpp status_;
    EnvDataCpp      env_;
    int run_mode_      = 0;
    int power_status_  = 0;
};

#endif /* DEV_AIR_PURIFIER_HPP */