#ifndef DEV_ALARM_DEVICE_H
#define DEV_ALARM_DEVICE_H

/**
 * @file dev_alarm_device.h
 * @brief 设备抽象层 - 报警装置 (驱鼠器, Alarm Device)
 *
 * 设备地址: 0x80 (128)
 * 功能: 线圈读取(0x01)、线圈写入(0x05)
 *   查询继电器吸合/断开状态
 *   控制继电器吸合(0xFF00)/断开(0x0000)
 *
 * API命名遵循Word文档规范:
 *   ntfAlarmState() / ntfAlarmStateProc() - 读取/处理报警器状态
 *   setAlarmOn()   - 开启报警器 (继电器吸合)
 *   setAlarmOff()  - 关闭报警器 (继电器断开)
 */

#include <stdint.h>
#include <stddef.h>
#include "dev_common.h"
#include "service/modbus_service.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    DeviceStatus status;
    int          state;  /* 报警器状态: 0=关闭, 1=开启 */
} DevAlarmData;

extern DevAlarmData g_dev_alarm_data;

void init_dev_alarm_data();

/* ============================================================
 * 读取API (ntf*)
 * ============================================================ */
int ntfAlarmState(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/* ============================================================
 * 数据处理API (ntf*Proc)
 * ============================================================ */
void ntfAlarmStateProc(const uint8_t *resp, size_t resp_len, int rc);

/* ============================================================
 * 控制API (set*)
 * ============================================================ */
int setAlarmOn(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int setAlarmOff(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

#ifdef __cplusplus
}
#endif

#endif /* DEV_ALARM_DEVICE_H */