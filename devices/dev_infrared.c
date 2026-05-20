#include "dev_infrared.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevInfraredData g_dev_infrared_data;

void init_dev_infrared_data() {
    memset(&g_dev_infrared_data, 0, sizeof(DevInfraredData));
    dev_status_init(&g_dev_infrared_data.status);
}

int ntfIR(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_INFRARED_ADDR, REG_INFRARED_STATE,
                         REG_INFRARED_DATA, resp, 512, resp_len);
}

void ntfIRProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        if (dev_status_on_failure(&g_dev_infrared_data.status, 3)) {
            printf("  => [⚠️ 设备离线]: 红外探测器连续3次未读到数据\n");
        }
        return;
    }
    dev_status_on_success(&g_dev_infrared_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_INFRARED_ADDR, 0x03, 2);
    if (parse_rc == PARSE_OK) {
        uint16_t val = parseU16(resp, 0);
        g_dev_infrared_data.infrared_state = val;

        if (val == 0) {
            printf("  => [✅ 红外状态]: 正常，未检测到人体入侵\n");
        } else if (val == 1) {
            printf("  => [🚨 红外报警]: 检测到有人体入侵！\n");
        } else {
            printf("  => [⚠️ 未知状态]: 收到非标准红外状态码: %d\n", val);
        }
    } else if (parse_rc == PARSE_ERR_CRC) {
        printf("  => [❌ 解析失败]: 红外数据 CRC校验位错误\n");
    } else {
        printf("  => [❌ 解析失败]: 红外响应格式不符合预期协议\n");
    }
}