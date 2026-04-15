#include "dev_light.h"
#include "device_config.h"
#include "modbus_core.h"
#include "sensor_data.h"
#include <stdio.h>

int light_read_illuminance(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_LIGHT_ADDR, 0x03, REG_LIGHT_ILLUMIN, 0x0001, resp, 512, resp_len, 300);
}

// 专门处理接收到的光感数据: e.g. 50 03 02 01 33 04 0D
void light_process_illuminance_data(const uint8_t *resp, size_t resp_len) {
    // 包结构：
    // resp[0] = 0x50 (设备地址)
    // resp[1] = 0x03 (功能码)
    // resp[2] = 0x02 (返回有效字节数，也就是2个字节的数据)
    // resp[3], resp[4] 是测得的光照度的高/低位
    // 后两位是CRC校验位

    if (resp_len >= 7 && resp[0] == DEV_LIGHT_ADDR && resp[1] == 0x03 && resp[2] == 0x02) {
        
        // 1. 进行 CRC16 校验 (倒数第二位和第一位)
        uint16_t expected_crc = crc16_modbus(resp, resp_len - 2);
        uint16_t actual_crc = resp[resp_len - 2] | (resp[resp_len - 1] << 8);
        if (expected_crc != actual_crc) {
             printf("  => [❌ 解析失败]: 光感数据 CRC 校验位错误\n");
             return;
        }

        // 2. 取出高低位数据拼接
        // 例如 01 33 转换 => 0x0133
        uint16_t illuminance_hex = (resp[3] << 8) | resp[4];
        
        pthread_mutex_lock(&g_sensor_data.lock);
        g_sensor_data.lux = illuminance_hex;
        pthread_mutex_unlock(&g_sensor_data.lock);

        // C语言中 %u(%d) 格式化打印就是十进制 (0x0133 的十进制即为 307); 单位为 Lux
        printf("  => [☀️ 照度详情]: 探测当前光照度为 %u lux\n", illuminance_hex);

    } else {
        printf("  => [❌ 解析失败]: 响应格式不符合预期协议或地址不匹配\n");
    }
}