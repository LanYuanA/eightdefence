/**
 * @file dev_air_conditioner.hpp
 * @brief 设备抽象层 - 空调集中控制器 (C++)
 */

#ifndef DEV_AIR_CONDITIONER_HPP
#define DEV_AIR_CONDITIONER_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"

class DevAirConditioner : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "空调集中控制器"; }

    int setCoolOn(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    int setCoolOff(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    int setHeatOn(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    int setHeatOff(ModbusService &svc, uint8_t *resp, size_t *resp_len);

    void procACResponse(const uint8_t *resp, size_t resp_len, int rc, const char *action);

    bool isOnline() { return status_.isOnline(); }

private:
    DeviceStatusCpp status_;
};

#endif /* DEV_AIR_CONDITIONER_HPP */