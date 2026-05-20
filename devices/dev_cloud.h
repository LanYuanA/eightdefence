#ifndef DEV_CLOUD_H
#define DEV_CLOUD_H

/**
 * @file dev_cloud.h
 * @brief 设备抽象层 - 室内空气质量变送器 (云测仪, Air Quality Sensor)
 *
 * 设备地址: 0x30 (48)
 * 支持读取: PM2.5, PM10, 湿度, 温度, TVOC, 甲醛, O3, CO2
 *
 * API命名遵循Word文档规范:
 *   ntfPM25() / ntfPM25Proc()     - PM2.5 数据
 *   ntfPM10() / ntfPM10Proc()     - PM10 数据
 *   ntfHmd()  / ntfHmdProc()      - 湿度数据
 *   ntfT()    / ntfTProc()        - 温度数据
 *   ntfTVOC() / ntfTVOCProc()     - TVOC 数据
 *   ntfCH2O() / ntfCH2OProc()     - 甲醛数据
 *   ntfO3()   / ntfO3Proc()       - 臭氧数据
 *   ntfCO2()  / ntfCO2Proc()      - CO2 数据
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
} DevCloudData;

extern DevCloudData g_dev_cloud_data;

void init_dev_cloud_data();

/* ============================================================
 * 读取API (ntf*) - 使用原子服务层的 ModbusContext
 * ============================================================ */
int ntfPM25(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfPM10(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfHmd(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfT(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfTVOC(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfCH2O(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfO3(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfCO2(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/* ============================================================
 * 数据处理API (ntf*Proc)
 * ============================================================ */
void ntfPM25Proc(const uint8_t *resp, size_t resp_len, int rc);
void ntfPM10Proc(const uint8_t *resp, size_t resp_len, int rc);
void ntfHmdProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfTProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfTVOCProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfCH2OProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfO3Proc(const uint8_t *resp, size_t resp_len, int rc);
void ntfCO2Proc(const uint8_t *resp, size_t resp_len, int rc);

#ifdef __cplusplus
}
#endif

#endif /* DEV_CLOUD_H */