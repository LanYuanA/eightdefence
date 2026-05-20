#include "dev_water.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevWaterData g_dev_water_data;

void init_dev_water_data() {
    memset(&g_dev_water_data, 0, sizeof(DevWaterData));
    dev_status_init(&g_dev_water_data.status);
}

int ntfWater(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_WATER_ADDR, REG_WATER_STATUS,
                         REG_WATER_DATA, resp, 512, resp_len);
}

void ntfWaterProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        if (dev_status_on_failure(&g_dev_water_data.status, 3)) {
            printf("  => [⚠️ 设备离线]: 水浸传感器连续3次未读到数据\n");
        }
        return;
    }
    dev_status_on_success(&g_dev_water_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_WATER_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        g_dev_water_data.water_state = (val != 0) ? 1 : 0;

        if (val == 0) {
            printf("  => [✅ 水浸状态]: 正常，未检测到水浸\n");
        } else if (val == 1) {
            printf("  => [🚨 水浸报警]: 检测到有水浸入！\n");
        } else {
            printf("  => [⚠️ 未知状态]: 收到非标准水浸状态码: %d\n", val);
        }
    } else if (parse_rc == PARSE_ERR_CRC) {
        printf("  => [❌ 解析失败]: 水浸数据 CRC校验位错误\n");
    } else {
        printf("  => [❌ 解析失败]: 水浸响应格式不符合预期协议\n");
    }
}