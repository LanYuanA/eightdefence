#ifndef DEV_HUMIDIFIER_H
#define DEV_HUMIDIFIER_H

/**
 * @file dev_humidifier.h
 * @brief 设备抽象层 - 恒湿净化一体机 (Humidifier)
 *
 * 设备地址: 0x20 (32)
 * 功能: 读取环境数据、设备状态，控制开关机、除湿、加湿、净化、恒湿
 *
 * API命名遵循Word文档规范:
 *   ntfHmdEnvData() / ntfHmdEnvDataProc()  - 读取/处理环境数据
 *   ntfHmdPowerSt() / ntfHmdPowerStProc()  - 读取/处理电源状态
 *   ntfHmdFltSt()   / ntfHmdFltStProc()    - 读取/处理故障状态
 *   setHmdPower()    - 设置开关机
 *   setHmdDehumidify()- 设置除湿
 *   setHmdHumidify() - 设置加湿
 *   setHmdPurify()   - 设置净化
 *   setHmdConstHum() - 设置恒湿
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
    int          power_state;       /* 开关机状态 */
    int          dehumidify_state;  /* 除湿状态 */
    int          humidify_state;    /* 加湿状态 */
    int          purify_state;      /* 净化状态 */
    int          const_hum_state;   /* 恒湿状态 */
    int          fault_state;       /* 故障状态 */
} DevHumidifierData;

extern DevHumidifierData g_dev_humidifier_data;

void init_dev_humidifier_data();

/* ============================================================
 * 读取API (ntf*)
 * ============================================================ */
int ntfHmdEnvData(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfHmdPowerSt(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
int ntfHmdFltSt(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);

/* ============================================================
 * 数据处理API (ntf*Proc)
 * ============================================================ */
void ntfHmdEnvDataProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfHmdPowerStProc(const uint8_t *resp, size_t resp_len, int rc);
void ntfHmdFltStProc(const uint8_t *resp, size_t resp_len, int rc);

/* ============================================================
 * 控制API (set*)
 * ============================================================ */
int setHmdPower(ModbusContext *ctx, uint16_t value,
                uint8_t *resp, size_t *resp_len);
int setHmdDehumidify(ModbusContext *ctx, uint16_t value,
                     uint8_t *resp, size_t *resp_len);
int setHmdHumidify(ModbusContext *ctx, uint16_t value,
                   uint8_t *resp, size_t *resp_len);
int setHmdPurify(ModbusContext *ctx, uint16_t value,
                 uint8_t *resp, size_t *resp_len);
int setHmdConstHum(ModbusContext *ctx, uint16_t value,
                   uint8_t *resp, size_t *resp_len);

#ifdef __cplusplus
}
#endif

#endif /* DEV_HUMIDIFIER_H */