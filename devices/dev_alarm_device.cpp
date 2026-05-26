/**
 * @file dev_alarm_device.cpp
 * @brief 设备抽象层 - 报警装置(驱鼠器)实现 (C++)
 */

#include "dev_alarm_device.hpp"
#include "device_config.h"
#include "service/parse_service.hpp"

#include <cstdio>
#include <cstring>

void DevAlarmDevice::init() {
    status_.reset();
    state_.store(0);
}

std::vector<DeviceTask> DevAlarmDevice::getTasks() {
    std::vector<DeviceTask> tasks;

    tasks.push_back({
        "报警装置 - 状态",
        [this](ModbusService &s, uint8_t *r, size_t *l) { return readState(s, r, l); },
        [this](const uint8_t *r, size_t l, int rc) { procState(r, l, rc); },
        5000, 1000, DEV_ALARM_ADDR
    });

    return tasks;
}

int DevAlarmDevice::readState(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.readCoil(DEV_ALARM_ADDR, REG_ALARM_COIL, REG_ALARM_DATA,
                         resp, 512, resp_len);
}

void DevAlarmDevice::procState(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        if (status_.onFailure(3)) {
            printf("  => [⚠️ 设备离线]: 报警装置(驱鼠器)连续3次未读到数据\n");
        }
        return;
    }
    status_.onSuccess();

    /* 线圈读取响应: 字节3为数据字节, bit0表示线圈状态 */
    if (resp_len >= 4) {
        int state = resp[3] & 0x01;
        state_.store(state);
    } else {
        printf("  => [❌ 解析失败]: 报警装置响应数据长度不足\n");
    }
}

int DevAlarmDevice::setAlarmOn(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.writeCoil(DEV_ALARM_ADDR, REG_ALARM_COIL, true, resp, 512, resp_len);
}

int DevAlarmDevice::setAlarmOff(ModbusService &svc, uint8_t *resp, size_t *resp_len) {
    return svc.writeCoil(DEV_ALARM_ADDR, REG_ALARM_COIL, false, resp, 512, resp_len);
}