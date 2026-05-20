/**
 * @file dev_light.hpp
 * @brief 设备抽象层 - 弱光检测传感器/光感 (C++)
 */

#ifndef DEV_LIGHT_HPP
#define DEV_LIGHT_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"

class DevLight : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "弱光检测传感器"; }

    int readLight(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procLight(const uint8_t *resp, size_t resp_len, int rc);

    uint16_t getIlluminance() const { return illuminance_; }
    bool isOnline() { return status_.isOnline(); }

private:
    DeviceStatusCpp status_;
    uint16_t illuminance_ = 0;
};

#endif /* DEV_LIGHT_HPP */