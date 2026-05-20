#ifndef DEV_LIGHT_H
#define DEV_LIGHT_H

/**
 * @file dev_light.h
 * @brief 设备抽象层 - 弱光检测传感器 / 光感 (Light Sensor)
 *
 * 设备地址: 0x50 (80)
 * 功能: 读取光照强度 (1个寄存器)
 *
 * API命名遵循Word文档规范:
 *   ntfLight()     - 读取光照强度 (Modbus请求)
 *   ntfLightProc() - 处理光照强度响应数据
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
    int          illuminance;  /* 光照强度 */
} DevLightData;

extern DevLightData g_dev_light_data;

void init_dev_light_data();

/**
 * @brief 读取光照强度 (ntfLight)
 * @return 0=成功, 其他=失败
 */
int ntfLight(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/**
 * @brief 处理光照强度响应数据 (ntfLightProc)
 */
void ntfLightProc(const uint8_t *resp, size_t resp_len, int rc);

#ifdef __cplusplus
}
#endif

#endif /* DEV_LIGHT_H */