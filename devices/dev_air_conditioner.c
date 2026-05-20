#include "dev_air_conditioner.h"
#include "device_config.h"
#include "service/parse_service.h"

#include <stdio.h>
#include <string.h>

DevACData g_dev_ac_data;

void init_dev_ac_data() {
    memset(&g_dev_ac_data, 0, sizeof(DevACData));
    dev_status_init(&g_dev_ac_data.status);
}

/* ============================================================
 * 控制API实现 (set*)
 * ============================================================ */
int setACCoolOn(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_AC_ADDR, REG_AC_COOL_ON,
                          0x0001, resp, 512, resp_len);
}

int setACCoolOff(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_AC_ADDR, REG_AC_COOL_OFF,
                          0x0001, resp, 512, resp_len);
}

int setACHeatOn(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_AC_ADDR, REG_AC_HEAT_ON,
                          0x0001, resp, 512, resp_len);
}

int setACHeatOff(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusWriteReg(ctx, DEV_AC_ADDR, REG_AC_HEAT_OFF,
                          0x0001, resp, 512, resp_len);
}

/* ============================================================
 * 响应处理API实现 (ntfACProc)
 * ============================================================ */
void ntfACProc(const uint8_t *resp, size_t resp_len, int rc,
               const char *action) {
    if (rc != 0) {
        if (dev_status_on_failure(&g_dev_ac_data.status, 3)) {
            printf("  => [⚠️ 设备离线]: 空调集中控制器连续3次未读到数据\n");
        }
        return;
    }
    dev_status_on_success(&g_dev_ac_data.status);

    int parse_rc = parseValidateResp(resp, resp_len, DEV_AC_ADDR, 0x06, 0);
    if (parse_rc == PARSE_OK) {
        printf("  => [❄️ 空调控制]: %s 操作成功\n", action);
    } else {
        printf("  => [❌ 空调控制]: %s 操作失败\n", action);
    }
}