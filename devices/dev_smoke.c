#include "dev_smoke.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevSmokeData g_dev_smoke_data;

void init_dev_smoke_data() {
    memset(&g_dev_smoke_data, 0, sizeof(DevSmokeData));
    dev_status_init(&g_dev_smoke_data.status);
}

int ntfSmoke(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadReg(ctx, DEV_SMOKE_ADDR, REG_SMOKE_ALARM,
                         REG_SMOKE_DATA, resp, 512, resp_len);
}

void ntfSmokeProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        if (dev_status_on_failure(&g_dev_smoke_data.status, 3)) {
            printf("  => [⚠️ 设备离线]: 烟雾报警器连续3次未读到数据\n");
        }
        return;
    }
    dev_status_on_success(&g_dev_smoke_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_SMOKE_ADDR, 0x03, 0);
    if (parse_rc == PARSE_OK) {
        uint16_t data = parseU16(resp, 0);
        g_dev_smoke_data.alarm_state = (data != 0) ? 1 : 0;

        if (data == 0x0000) {
            printf("  => [✅ 烟雾状态]: 正常，未发现烟雾\n");
        } else if (data == 0x0001) {
            printf("  => [🚨 烟雾报警]: 检测到有烟雾入侵！\n");
        } else {
            printf("  => [⚠️ 未知状态]: 收到非标准烟雾状态码: %d\n", data);
        }
    } else if (parse_rc == PARSE_ERR_CRC) {
        printf("  => [❌ 解析失败]: 烟雾数据 CRC校验位错误\n");
    } else {
        printf("  => [❌ 解析失败]: 烟雾响应格式不符合预期协议\n");
    }
}