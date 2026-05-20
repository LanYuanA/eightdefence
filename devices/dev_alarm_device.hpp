/**
 * @file dev_alarm_device.hpp
 * @brief 设备抽象层 - 报警装置(驱鼠器) (C++)
 */

#ifndef DEV_ALARM_DEVICE_HPP
#define DEV_ALARM_DEVICE_HPP

#include "device_base.hpp"
#include "service/parse_service.hpp"

class DevAlarmDevice : public DeviceBase {
public:
    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return "报警装置"; }

    int readState(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    void procState(const uint8_t *resp, size_t resp_len, int rc);

    // 控制命令
    int setAlarmOn(ModbusService &svc, uint8_t *resp, size_t *resp_len);
    int setAlarmOff(ModbusService &svc, uint8_t *resp, size_t *resp_len);

    bool isOnline() { return status_.isOnline(); }
    int getState() { return state_; }

private:
    DeviceStatusCpp status_;
    int state_ = 0;  // 0=关闭, 1=开启
};

#endif /* DEV_ALARM_DEVICE_HPP */