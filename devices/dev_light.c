#include "dev_light.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevLightData g_dev_light_data;

void init_dev_light_data() {
    memset(&g_dev_light_data, 0, sizeof(DevLightData));
    dev_status_init(&g_dev_light_data.status);
}

int ntfLight(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_LIGHT_ADDR, REG_LIGHT_ILLUMIN,
                         REG_LIGHT_DATA, resp, 512, resp_len);
}

void ntfLightProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        if (dev_status_on_failure(&g_dev_light_data.status, 3)) {
            printf("  => [⚠️ 设备离线]: 光感传感器连续3次未读到数据\n");
        }
        return;
    }
    dev_status_on_success(&g_dev_light_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_LIGHT_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        g_dev_light_data.illuminance = val;
        printf("  => [💡 光照详情]: 光照强度为 %d Lux\n", val);
    } else if (parse_rc == PARSE_ERR_CRC) {
        printf("  => [❌ 解析失败]: 光照数据 CRC校验位错误\n");
    } else {
        printf("  => [❌ 解析失败]: 光照响应格式不符合预期协议\n");
    }
}