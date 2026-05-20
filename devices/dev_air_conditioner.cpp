/**
 * @file dev_air_conditioner.cpp
 * @brief 设备抽象层 - 空调集中控制器实现 (C++)
 */

#include "dev_air_conditioner.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>
#include <cstring>

void DevAirConditioner::init() {
    status_.reset();
}

std::vector<DeviceTask> DevAirConditioner::getTasks() {
    // 空调集中控制器只通过HTTP API控制，没有轮询任务
    return {};
}

int DevAirConditioner::setCoolOn(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_AC_ADDR, REG_AC_COOL_ON, 0x0001, resp, 512, resp_len);
}

int DevAirConditioner::setCoolOff(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_AC_ADDR, REG_AC_COOL_OFF, 0x0001, resp, 512, resp_len);
}

int DevAirConditioner::setHeatOn(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_AC_ADDR, REG_AC_HEAT_ON, 0x0001, resp, 512, resp_len);
}

int DevAirConditioner::setHeatOff(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.writeReg(DEV_AC_ADDR, REG_AC_HEAT_OFF, 0x0001, resp, 512, resp_len);
}

void DevAirConditioner::procACResponse(const uint8_t *resp, size_t resp_len,
                                         int rc, const char *action) {
    if (rc != 0) {
        if (status_.onFailure(3)) {
            printf("  => [⚠️ 设备离线]: 空调集中控制器连续3次未读到数据\n");
        }
        return;
    }
    status_.onSuccess();

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_AC_ADDR, 0x06, 0);
    if (parse_rc == ParseService::OK) {
        printf("  => [❄️ 空调控制]: %s 操作成功\n", action);
    } else {
        printf("  => [❌ 空调控制]: %s 操作失败\n", action);
    }
}