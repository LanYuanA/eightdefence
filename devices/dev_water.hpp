/**
 * @file dev_water.hpp
 * @brief 设备抽象层 - 水浸传感器 (C++)
 */

#ifndef DEV_WATER_HPP
#define DEV_WATER_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"
#include <atomic>

class DevWater : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "水浸传感器"; }

    int readWater(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procWater(const uint8_t *resp, size_t resp_len, int rc);

    int getWaterState() const { return water_state_.load(); }
    bool isOnline() { return status_.isOnline(); }

private:
    DeviceStatusCpp status_;
    std::atomic<int> water_state_{0};
};

#endif /* DEV_WATER_HPP */