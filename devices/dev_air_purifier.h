#ifndef DEV_AIR_PURIFIER_H
#define DEV_AIR_PURIFIER_H

/**
 * @file dev_air_purifier.h
 * @brief 设备抽象层 - 霉菌空气净化机 (Air Purifier)
 *
 * 设备地址: 0x10 (16)
 * 功能: 读取环境数据、运行模式、电源状态，控制开关机、运行模式等
 *
 * API命名遵循Word文档规范:
 *   ntfPurEnvData() / ntfPurEnvDataProc()  - 读取/处理环境数据
 *   ntfPurRunMode() / ntfPurRunModeProc()  - 读取/处理运行模式
 *   ntfPurPowerSt() / ntfPurPowerStProc()  - 读取/处理电源状态
 *   setPurPower()     - 设置开关机
 *   setPurRunMode()   - 设置运行模式
 *   setPurManual()    - 设置手动模式
 *   setPurTiming()    - 设置定时模式
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
    EnvData      env;
    uint16_t     run_mode;       /* 运行模式 */
    uint16_t     power_status;   /* 电源状态 */
} DevPurifierData;

extern DevPurifierData g_dev_purifier_data;

void init_dev_purifier_data();

/* ============================================================
 * 读取API (ntf*)
 * ============================================================ */
int ntfPurEnvData(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfPurRunMode(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfPurPowerSt(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/* ============================================================
 * 数据处理API (ntf*Proc)
 * ============================================================ */
void ntfPurEnvDataProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfPurRunModeProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfPurPowerStProc(const uint8_t *resp, size_t resp_len, int rc);

/* ============================================================
 * 控制API (set*)
 * ============================================================ */
int setPurPower(ModbusContext *ctx, uint16_t value,
                uint8_t *resp, size_t *resp_len);
int setPurRunMode(ModbusContext *ctx, uint16_t value,
                  uint8_t *resp, size_t *resp_len);
int setPurManual(ModbusContext *ctx, uint16_t value,
                 uint8_t *resp, size_t *resp_len);
int setPurTiming(ModbusContext *ctx, uint16_t value,
                 uint8_t *resp, size_t *resp_len);

#ifdef __cplusplus
}
#endif

#endif /* DEV_AIR_PURIFIER_H */