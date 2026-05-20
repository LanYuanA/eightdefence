#ifndef DEV_AIR_CONDITIONER_H
#define DEV_AIR_CONDITIONER_H

/**
 * @file dev_air_conditioner.h
 * @brief 设备抽象层 - 空调集中控制器 (Air Conditioner Controller)
 *
 * 设备地址: 0x60 (96)
 * 功能: 制冷开/关、制热开/关 (0x0001触发)
 *
 * API命名遵循Word文档规范:
 *   ntfACProc()    - 处理空调控制响应数据
 *   setACCoolOn()  - 制冷开
 *   setACCoolOff() - 制冷关
 *   setACHeatOn()  - 制热开
 *   setACHeatOff() - 制热关
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
} DevACData;

extern DevACData g_dev_ac_data;

void init_dev_ac_data();

/* ============================================================
 * 控制API (set*)
 * ============================================================ */
int setACCoolOn(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int setACCoolOff(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int setACHeatOn(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int setACHeatOff(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/**
 * @brief 处理空调控制响应数据 (ntfACProc)
 */
void ntfACProc(const uint8_t *resp, size_t resp_len, int rc,
               const char *action);

#ifdef __cplusplus
}
#endif

#endif /* DEV_AIR_CONDITIONER_H */