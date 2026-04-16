#include "dev_water.h"
#include "device_config.h"
#include "modbus_core.h"

#include <stdio.h>
#include <string.h>

DevWaterData g_dev_water_data;

void init_dev_water_data() {
    memset(&g_dev_water_data, 0, sizeof(DevWaterData));
    pthread_mutex_init(&g_dev_water_data.lock, NULL);
}


int water_read_status(const char* device, uint8_t *resp, size_t *resp_len) {
    // 假设宏定义依然被某处更改了，我用最兼容的硬编码或原样发送
    // 此处保持了用户原来的变量，增加了解析函数
    return modbus_build_and_send(device, 9600, DEV_WATER_ADDR, 0x03, REG_WATER_STATUS, 0x0001, resp, 512, resp_len, 300);
}

// 解析收到的16进制完整串
int water_parse_data(const uint8_t *resp, size_t resp_len, int *is_immersed) {
    // 根据协议：应答是 90 03 02 00 00 45 99
    // resp[0] = 0x90 (地址位)
    // resp[1] = 0x03 (功能码)
    // resp[2] = 0x02 (返回有效字节数)
    // resp[3] 和 resp[4] 是数据高低位
    // resp[5] 和 resp[6] 是 CRC 校验码 (低位在前、高位在后)

    if (resp_len >= 7 && resp[0] == DEV_WATER_ADDR && resp[1] == 0x03 && resp[2] == 0x02) {
        
        // 1. 进行 CRC16 校验
        uint16_t expected_crc = crc16_modbus(resp, resp_len - 2);
        uint16_t actual_crc = resp[resp_len - 2] | (resp[resp_len - 1] << 8);
        if (expected_crc != actual_crc) {
             // CRC 错误
             return -2;
        }

        // 2. 取出两位数据拼接
        uint16_t data = (resp[3] << 8) | resp[4];
        
        if (data == 0x0000) {
            *is_immersed = 0; // 无浸水
        } else if (data == 0x0001) {
            *is_immersed = 1; // 有浸水
        } else {
            *is_immersed = data; // 其它未知状态
        }
        
        return 0; // 成功解析
    }
    return -1; // 协议格式或地址不符，解析失败
}

// =========================================
// 处理当前传感器传回数据的打印和后续业务逻辑
// =========================================
void water_process_data(const uint8_t *resp, size_t resp_len, int rc) {
    pthread_mutex_lock(&g_dev_water_data.lock);
    if (rc != 0) {
        g_dev_water_data.fail_count++;
        if (g_dev_water_data.fail_count >= 3) {
            g_dev_water_data.online = 0;
            printf("  => [⚠️ 设备离线]: 水浸传感器连续3次未读到数据\n");
        }
        pthread_mutex_unlock(&g_dev_water_data.lock);
        return;
    }
    g_dev_water_data.fail_count = 0;
    g_dev_water_data.online = 1;
    pthread_mutex_unlock(&g_dev_water_data.lock);

    int is_immersed = 0;
    int parse_rc = water_parse_data(resp, resp_len, &is_immersed);
    
    if (parse_rc == 0) {
        pthread_mutex_lock(&g_dev_water_data.lock);
        g_dev_water_data.water_status = is_immersed;
        pthread_mutex_unlock(&g_dev_water_data.lock);

        if (is_immersed == 1) {
            printf("  => [🚨 水浸报警]: 检测到有溢水/漏水情况发生！\n");
        } else if (is_immersed == 0) {
            printf("  => [✅ 水浸状态]: 正常 (目前未发现积水)\n");
        } else {
            printf("  => [⚠️ 未知状态]: 收到非标准状态码: %d\n", is_immersed);
        }
    } else if (parse_rc == -2) {
        printf("  => [❌ 解析失败]: CRC校验位错误，通信数据可能已损坏！\n");
    } else {
        printf("  => [❌ 解析失败]: 响应格式不符合预期协议\n");
    }
}