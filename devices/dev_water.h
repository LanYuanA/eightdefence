#ifndef DEV_WATER_H
#define DEV_WATER_H

/**
 * @file dev_water.h
 * @brief 设备抽象层 - 水浸传感器 (Water Sensor)
 *
 * 设备地址: 0x90 (144)
 * 功能: 读取水浸状态 (1个寄存器)
 *
 * API命名遵循Word文档规范:
 *   ntfWater()     - 读取水浸状态 (Modbus请求)
 *   ntfWaterProc() - 处理水浸状态响应数据
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
    int          water_state;  /* 1=有水浸, 0=正常 */
} DevWaterData;

extern DevWaterData g_dev_water_data;

void init_dev_water_data();

/**
 * @brief 读取水浸传感器状态 (ntfWater)
 * @return 0=成功, 其他=失败
 */
int ntfWater(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/**
 * @brief 处理水浸传感器响应数据 (ntfWaterProc)
 */
void ntfWaterProc(const uint8_t *resp, size_t resp_len, int rc);

#ifdef __cplusplus
}
#endif

#endif /* DEV_WATER_H */