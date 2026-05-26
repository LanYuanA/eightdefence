/**
 * @file dev_air_purifier.hpp
 * @brief 设备抽象层 - 霉菌空气净化机 (C++) — 执行器 + 可选读取
 */

#ifndef DEV_AIR_PURIFIER_HPP
#define DEV_AIR_PURIFIER_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"
#include <atomic>

class DevAirPurifier : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "霉菌空气净化机"; }

    // 读取指令（封装但暂不轮询）
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

    int getRunMode() const { return run_mode_.load(); }
    int getPowerStatus() const { return power_status_.load(); }
    bool isOnline() { return status_.isOnline(); }
    EnvDataCpp getEnvData() const { return env_; }

private:
    DeviceStatusCpp status_;
    EnvDataCpp      env_;
    std::atomic<int> run_mode_{0};
    std::atomic<int> power_status_{0};
};

#endif /* DEV_AIR_PURIFIER_HPP */
