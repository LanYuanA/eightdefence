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
    return svc.readReg(DEV_AC_ADDR, REG_AC_COOL_ON, REG_AC_DATA, resp, 512, resp_len);
}

int DevAirConditioner::setCoolOff(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_AC_ADDR, REG_AC_COOL_OFF, REG_AC_DATA, resp, 512, resp_len);
}

int DevAirConditioner::setHeatOn(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_AC_ADDR, REG_AC_HEAT_ON, REG_AC_DATA, resp, 512, resp_len);
}

int DevAirConditioner::setHeatOff(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readReg(DEV_AC_ADDR, REG_AC_HEAT_OFF, REG_AC_DATA, resp, 512, resp_len);
}

void DevAirConditioner::procACResponse(const uint8_t *resp, size_t resp_len,
                                         int rc, const char *action) {
    if (rc != 0) {
        if (status_.onFailure(3)) {
            printf("  => [⚠️ 设备离线]: 空调集中控制器连续3次未读到数据\n");
        }
        return;
    }

    int parse_rc = ParseService::parseDeviceData(resp, resp_len, DEV_AC_ADDR, 0x03, 0);
    if (parse_rc != ParseService::OK) {
        printf("  => [❌ 空调控制]: %s 操作失败 (期望地址0x%02X)\n", action, DEV_AC_ADDR);
        if (resp_len > 0) {
            printf("     原始数据[%zu字节]: ", resp_len);
            size_t dump_len = resp_len > 32 ? 32 : resp_len;
            for (size_t i = 0; i < dump_len; i++) printf("%02X ", resp[i]);
            printf("\n");
        }
        return;
    }

    status_.onSuccess();
    printf("  => [❄️ 空调控制]: %s 操作成功\n", action);
}