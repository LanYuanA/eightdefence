#include "dev_cloud.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevCloudData g_dev_cloud_data;

void init_dev_cloud_data() {
    memset(&g_dev_cloud_data, 0, sizeof(DevCloudData));
    dev_status_init(&g_dev_cloud_data.status);
    env_data_init(&g_dev_cloud_data.env);
}

/* ============================================================
 * 读取API实现 (ntf*)
 * ============================================================ */
int ntfPM25(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_PM25,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

int ntfPM10(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_PM10,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

int ntfHmd(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_HUMIDITY,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

int ntfT(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_TEMP,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

int ntfTVOC(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_TVOC,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

int ntfCH2O(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_CH2O,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

int ntfO3(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_O3,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

int ntfCO2(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_CLOUD_ADDR, REG_CLOUD_CO2,
                         REG_CLOUD_DATA, resp, 512, resp_len);
}

/* ============================================================
 * 数据处理辅助函数
 * ============================================================ */
static void handle_cloud_failure(const char *name) {
    if (dev_status_on_failure(&g_dev_cloud_data.status, 3)) {
        printf("  => [⚠️ 设备离线]: %s 连续3次未读到数据\n", name);
    }
}

static void handle_cloud_parse_error(const char *name, int parse_rc) {
    if (parse_rc == PARSE_ERR_CRC) {
        printf("  => [❌ 解析失败]: %s CRC校验位错误\n", name);
    } else if (parse_rc == PARSE_ERR_ADDR) {
        printf("  => [❌ 解析失败]: %s 设备地址不匹配\n", name);
    } else {
        printf("  => [❌ 解析失败]: %s 响应格式不符合预期协议\n", name);
    }
}

/* ============================================================
 * 数据处理API实现 (ntf*Proc)
 * ============================================================ */
void ntfPM25Proc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("PM2.5"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.pm25 = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [📊 PM2.5详情]: PM2.5值为 %d ug/m3\n", val);
    } else {
        handle_cloud_parse_error("PM2.5", parse_rc);
    }
}

void ntfPM10Proc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("PM10"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.pm10 = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [📊 PM10详情]: PM10值为 %d ug/m3\n", val);
    } else {
        handle_cloud_parse_error("PM10", parse_rc);
    }
}

void ntfHmdProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("湿度"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.humidity = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [💧 湿度详情]: 湿度为 %d.%d%%RH\n", val / 10, val % 10);
    } else {
        handle_cloud_parse_error("湿度", parse_rc);
    }
}

void ntfTProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("温度"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        int16_t val = parseS16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.temperature = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [🌡️ 温度详情]: 温度为 %d.%d℃\n", val / 10, val % 10);
    } else {
        handle_cloud_parse_error("温度", parse_rc);
    }
}

void ntfTVOCProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("TVOC"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.tvoc = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [💨 TVOC详情]: TVOC值为 %d ppb\n", val);
    } else {
        handle_cloud_parse_error("TVOC", parse_rc);
    }
}

void ntfCH2OProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("甲醛"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.ch2o = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [🧪 甲醛详情]: 甲醛值为 %d mg/m3\n", val);
    } else {
        handle_cloud_parse_error("甲醛", parse_rc);
    }
}

void ntfO3Proc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("臭氧"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.o3 = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [🌬️ 臭氧详情]: 臭氧值为 %d ppb\n", val);
    } else {
        handle_cloud_parse_error("臭氧", parse_rc);
    }
}

void ntfCO2Proc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) { handle_cloud_failure("CO2"); return; }
    dev_status_on_success(&g_dev_cloud_data.status);
    int parse_rc = parseValidateResp(resp, resp_len, DEV_CLOUD_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        pthread_mutex_lock(&g_dev_cloud_data.env.lock);
        g_dev_cloud_data.env.co2 = val;
        pthread_mutex_unlock(&g_dev_cloud_data.env.lock);
        printf("  => [☁️ CO2详情]: CO2值为 %d ppm\n", val);
    } else {
        handle_cloud_parse_error("CO2", parse_rc);
    }
}