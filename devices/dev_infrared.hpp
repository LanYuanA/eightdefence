/**
 * @file dev_infrared.hpp
 * @brief 设备抽象层 - 红外探测器 (C++)
 */

#ifndef DEV_INFRARED_HPP
#define DEV_INFRARED_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"

class DevInfrared : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "红外探测器"; }

    int readInfrared(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procInfrared(const uint8_t *resp, size_t resp_len, int rc);

    int readRadar(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procRadar(const uint8_t *resp, size_t resp_len, int rc);

    int getInfraredState() const { return infrared_state_; }
    int getRadarState() const { return radar_state_; }
    bool isOnline() { return status_.isOnline(); }

private:
    DeviceStatusCpp status_;
    int infrared_state_ = 0;
    int radar_state_    = 0;
};

#endif /* DEV_INFRARED_HPP */