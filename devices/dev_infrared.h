#ifndef DEV_INFRARED_H
#define DEV_INFRARED_H

/**
 * @file dev_infrared.h
 * @brief 设备抽象层 - 红外探测器 (Infrared Sensor)
 *
 * 设备地址: 0x40 (64)
 * 功能: 读取红外状态和雷达状态 (2个寄存器)
 *
 * API命名遵循Word文档规范:
 *   ntfIR()      - 读取红外/雷达状态 (Modbus请求)
 *   ntfIRProc()  - 处理红外/雷达状态响应数据
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
    int          infrared_state;  /* 红外状态 */
    int          radar_state;     /* 雷达状态 */
} DevInfraredData;

extern DevInfraredData g_dev_infrared_data;

void init_dev_infrared_data();

/**
 * @brief 读取红外探测器状态 (ntfIR)
 * @return 0=成功, 其他=失败
 */
int ntfIR(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/**
 * @brief 处理红外探测器响应数据 (ntfIRProc)
 */
void ntfIRProc(const uint8_t *resp, size_t resp_len, int rc);

#ifdef __cplusplus
}
#endif

#endif /* DEV_INFRARED_H */