#ifndef DEV_SMOKE_H
#define DEV_SMOKE_H

/**
 * @file dev_smoke.h
 * @brief 设备抽象层 - 烟雾报警器 (Smoke Sensor)
 *
 * 设备地址: 0x70 (112)
 * 功能: 读取报警器状态 (10个寄存器)
 *
 * API命名遵循Word文档规范:
 *   ntfSmoke()     - 读取烟雾状态 (Modbus请求)
 *   ntfSmokeProc() - 处理烟雾状态响应数据
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
    int          alarm_state;  /* 1=报警, 0=正常 */
} DevSmokeData;

extern DevSmokeData g_dev_smoke_data;

void init_dev_smoke_data();

/**
 * @brief 读取烟雾报警器状态 (ntfSmoke)
 * @return 0=成功, 其他=失败
 */
int ntfSmoke(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/**
 * @brief 处理烟雾报警器响应数据 (ntfSmokeProc)
 */
void ntfSmokeProc(const uint8_t *resp, size_t resp_len, int rc);

#ifdef __cplusplus
}
#endif

#endif /* DEV_SMOKE_H */