#include "dev_alarm_device.h"
#include "device_config.h"

#include <stdio.h>
#include <string.h>

DevAlarmData g_dev_alarm_data;

void init_dev_alarm_data() {
    memset(&g_dev_alarm_data, 0, sizeof(DevAlarmData));
    dev_status_init(&g_dev_alarm_data.status);
}

/* ============================================================
 * 读取API实现 (ntf*)
 * ============================================================ */
int ntfAlarmState(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusReadCoil(ctx, DEV_ALARM_ADDR, REG_ALARM_COIL,
                          REG_ALARM_DATA, resp, 512, resp_len);
}

/* ============================================================
 * 数据处理API实现 (ntf*Proc)
 * ============================================================ */
void ntfAlarmStateProc(const uint8_t *resp, size_t resp_len, int rc) {
    if (rc != 0) {
        if (dev_status_on_failure(&g_dev_alarm_data.status, 3)) {
            printf("  => [⚠️ 设备离线]: 报警装置(驱鼠器)连续3次未读到数据\n");
        }
        return;
    }
    dev_status_on_success(&g_dev_alarm_data.status);

    /* 线圈读取响应: 字节3为数据字节, bit0表示线圈状态 */
    if (resp_len >= 4) {
        int state = resp[3] & 0x01;
        g_dev_alarm_data.state = state;
        printf("  => [🔔 报警装置]: 当前状态为 %s\n",
               state ? "开启(继电器吸合)" : "关闭(继电器断开)");
    } else {
        printf("  => [❌ 解析失败]: 报警装置响应数据长度不足\n");
    }
}

/* ============================================================
 * 控制API实现 (set*)
 * ============================================================ */
int setAlarmOn(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusWriteCoil(ctx, DEV_ALARM_ADDR, REG_ALARM_COIL,
                           REG_ALARM_ON, resp, 512, resp_len);
}

int setAlarmOff(ModbusContext *ctx, uint8_t *resp, size_t *resp_len) {
    return modbusWriteCoil(ctx, DEV_ALARM_ADDR, REG_ALARM_COIL,
                           REG_ALARM_OFF, resp, 512, resp_len);
}