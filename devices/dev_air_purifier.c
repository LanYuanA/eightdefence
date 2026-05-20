#include "dev_air_purifier.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevPurifierData g_dev_purifier_data;

void init_dev_purifier_data() {
    memset(&g_dev_purifier_data, 0, sizeof(DevPurifierData));
    dev_status_init(&g_dev_purifier_data.status);
    env_data_init(&g_dev_purifier_data.env);
}

/* ============================================================
 * 读取API实现 (ntf*)
 * ============================================================ */
int ntfPurEnvData(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_PURIFIER_ADDR, REG_PUR_CO2,
                         REG_PUR_READ_COUNT, resp, 512, resp_len);
}

int ntfPurRunMode(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_PURIFIER_ADDR, REG_PUR_RUN_MODE,
                         1, resp, 512, resp_len);
}

int ntfPurPowerSt(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_PURIFIER_ADDR, REG_PUR_POWER_STATUS,
                         1, resp, 512, resp_len);
}

/* ============================================================
 * 数据处理API实现 (ntf*Proc)
 * ============================================================ */
static void handle_purifier_failure(const char *name) {
    if (dev_status_on_failure(&g_dev_purifier_data.status, 3)) {
        printf("  => [⚠️ 设备离线]: 霉菌空气净化机(%s)连续3次未读到数据\n", name);
    }
}

void ntfPurEnvDataProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_purifier_failure("环境数据"); return; }
    dev_status_on_success(&g_dev_purifier_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_purifier_data.env.lock);
        g_dev_purifier_data.env.co2 = val;
        pthread_mutex_unlock(&g_dev_purifier_data.env.lock);
        printf("  => [📊 净化机环境]: CO2=%d\n", val);
    } else {
        printf("  => [❌ 解析失败]: 净化机环境数据 响应格式不符合预期协议\n");
    }
}

void ntfPurRunModeProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_purifier_failure("运行模式"); return; }
    dev_status_on_success(&g_dev_purifier_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        g_dev_purifier_data.run_mode = val;
        printf("  => [🔄 净化机模式]: 运行模式=%d\n", val);
    }
}

void ntfPurPowerStProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_purifier_failure("电源状态"); return; }
    dev_status_on_success(&g_dev_purifier_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_PURIFIER_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        g_dev_purifier_data.power_status = val;
        printf("  => [⚡ 净化机电源]: %s\n", val ? "开机" : "关机");
    }
}

/* ============================================================
 * 控制API实现 (set*)
 * ============================================================ */
int setPurPower(ModbusContext *ctx, uint16_t value,
                uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_PURIFIER_ADDR, REG_PUR_CTRL_POWER,
                          value, resp, 512, resp_len);
}

int setPurRunMode(ModbusContext *ctx, uint16_t value,
                  uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_PURIFIER_ADDR, REG_PUR_CTRL_RUN_MODE,
                          value, resp, 512, resp_len);
}

int setPurManual(ModbusContext *ctx, uint16_t value,
                 uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_PURIFIER_ADDR, REG_PUR_CTRL_MANUAL,
                          value, resp, 512, resp_len);
}

int setPurTiming(ModbusContext *ctx, uint16_t value,
                 uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_PURIFIER_ADDR, REG_PUR_CTRL_TIMING,
                          value, resp, 512, resp_len);
}