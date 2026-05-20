#include "dev_humidifier.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevHumidifierData g_dev_humidifier_data;

void init_dev_humidifier_data() {
    memset(&g_dev_humidifier_data, 0, sizeof(DevHumidifierData));
    dev_status_init(&g_dev_humidifier_data.status);
    env_data_init(&g_dev_humidifier_data.env);
}

/* ============================================================
 * 读取API实现 (ntf*)
 * ============================================================ */
int ntfHmdEnvData(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_CO2,
                         REG_HUM_READ_COUNT, resp, 512, resp_len);
}

int ntfHmdPowerSt(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_POWER_STATE,
                         1, resp, 512, resp_len);
}

int ntfHmdFltSt(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_FAULT_STATE,
                         1, resp, 512, resp_len);
}

/* ============================================================
 * 数据处理API实现 (ntf*Proc)
 * ============================================================ */
static void handle_humidifier_failure(const char *name) {
    if (dev_status_on_failure(&g_dev_humidifier_data.status, 3)) {
        printf("  => [⚠️ 设备离线]: 恒湿净化一体机(%s)连续3次未读到数据\n", name);
    }
}

void ntfHmdEnvDataProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_humidifier_failure("环境数据"); return; }
    dev_status_on_success(&g_dev_humidifier_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_HUMIDIFIER_ADDR, 0x03, 10);
    if (parse_rc == PARSE_OK) {
        pthread_mutex_lock(&g_dev_humidifier_data.env.lock);
        g_dev_humidifier_data.env.co2   = parseU16(resp, 0);
        g_dev_humidifier_data.env.ch2o  = parseU16(resp, 1);
        g_dev_humidifier_data.env.tvoc  = parseU16(resp, 2);
        g_dev_humidifier_data.env.pm25  = parseU16(resp, 3);
        g_dev_humidifier_data.env.pm10  = parseU16(resp, 4);
        pthread_mutex_unlock(&g_dev_humidifier_data.env.lock);
        printf("  => [📊 恒湿机环境]: CO2=%d, 甲醛=%d, TVOC=%d, PM2.5=%d, PM10=%d\n",
               g_dev_humidifier_data.env.co2, g_dev_humidifier_data.env.ch2o,
               g_dev_humidifier_data.env.tvoc, g_dev_humidifier_data.env.pm25,
               g_dev_humidifier_data.env.pm10);
    } else {
        printf("  => [❌ 解析失败]: 恒湿机环境数据 响应格式不符合预期协议\n");
    }
}

void ntfHmdPowerStProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_humidifier_failure("电源状态"); return; }
    dev_status_on_success(&g_dev_humidifier_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_HUMIDIFIER_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        g_dev_humidifier_data.power_state = val;
        printf("  => [⚡ 恒湿机电源]: %s\n", val ? "开机" : "关机");
    }
}

void ntfHmdFltStProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_humidifier_failure("故障状态"); return; }
    dev_status_on_success(&g_dev_humidifier_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_HUMIDIFIER_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        g_dev_humidifier_data.fault_state = val;
        if (val == 0) {
            printf("  => [✅ 恒湿机故障]: 正常，无故障\n");
        } else {
            printf("  => [🚨 恒湿机故障]: 故障码 0x%04X\n", val);
        }
    }
}

/* ============================================================
 * 控制API实现 (set*)
 * ============================================================ */
int setHmdPower(ModbusContext *ctx, uint16_t value,
                uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_POWER,
                          value, resp, 512, resp_len);
}

int setHmdDehumidify(ModbusContext *ctx, uint16_t value,
                     uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_DEHUMIDIFY,
                          value, resp, 512, resp_len);
}

int setHmdHumidify(ModbusContext *ctx, uint16_t value,
                   uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_HUMIDIFY,
                          value, resp, 512, resp_len);
}

int setHmdPurify(ModbusContext *ctx, uint16_t value,
                 uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_PURIFY,
                          value, resp, 512, resp_len);
}

int setHmdConstHum(ModbusContext *ctx, uint16_t value,
                   uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_CONST_HUM,
                          value, resp, 512, resp_len);
}