/**
 * @file dev_smoke.hpp
 * @brief 设备抽象层 - 烟雾报警器 (C++)
 */

#ifndef DEV_SMOKE_HPP
#define DEV_SMOKE_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"

class DevSmoke : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "烟雾报警器"; }

    int readSmoke(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procSmoke(const uint8_t *resp, size_t resp_len, int rc);

    int getAlarmState() const { return alarm_state_; }
    bool isOnline() { return status_.isOnline(); }

private:
    DeviceStatusCpp status_;
    int alarm_state_ = 0;
};

#endif /* DEV_SMOKE_HPP */