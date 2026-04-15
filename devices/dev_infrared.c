#include "dev_infrared.h"
#include "device_config.h"
#include "modbus_core.h"
#include "sensor_data.h"
#include <stdio.h>

// 利用模块内的全局静态变量临时记录单一传感器的最新状态 （0=无人 1=有人）
static int s_ir_state = 0;
static int s_radar_state = 0;

int infrared_read_state(const char* device, uint8_t *resp, size_t *resp_len) {
    // 兼容原有的宏
    return modbus_build_and_send(device, 9600, DEV_INFRARED_ADDR, 0x03, REG_INFRARED_STATE, 0x0001, resp, 512, resp_len, 300);
}

int radar_read_state(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_INFRARED_ADDR, 0x03, REG_RADAR_STATE, 0x0001, resp, 512, resp_len, 300);
}

// 统一的内部解析包协议代码，提取 0000(无人) 和 0001(有人)
static int parse_infrared_radar_data(const uint8_t *resp, size_t resp_len, int *state) {
    if (resp_len >= 7 && resp[0] == DEV_INFRARED_ADDR && resp[1] == 0x03 && resp[2] == 0x02) {
        // CRC校验
        uint16_t expected_crc = crc16_modbus(resp, resp_len - 2);
        uint16_t actual_crc = resp[resp_len - 2] | (resp[resp_len - 1] << 8);
        if (expected_crc != actual_crc) {
             return -2;
        }

        uint16_t data = (resp[3] << 8) | resp[4];
        *state = (data == 0x0001) ? 1 : 0;
        return 0;
    }
    return -1;
}

// 打印红外和雷达的综合探测逻辑
static void print_combined_detect_status() {
    // “只要有一个检测到人就说明有人” 逻辑
    if (s_ir_state == 1 || s_radar_state == 1) {
        printf("  => [🚨 综合判定]: 探测到活动人体！(红外: %s, 雷达: %s)\n", 
                s_ir_state ? "触发" : "未触发", 
                s_radar_state ? "触发" : "未触发");
    } else {
        printf("  => [✅ 综合判定]: 区域内安全，无人活动。\n");
    }
}

// 处理红外的响应数据
void infrared_process_ir_data(const uint8_t *resp, size_t resp_len) {
    int state = 0;
    int rc = parse_infrared_radar_data(resp, resp_len, &state);
    if (rc == 0) {
        s_ir_state = state;
        pthread_mutex_lock(&g_sensor_data.lock);
        g_sensor_data.ir_state = state;
        pthread_mutex_unlock(&g_sensor_data.lock);
        // 把结果交由综合判断控制打印
        print_combined_detect_status();
    } else if (rc == -2) {
        printf("  => [❌ 解析失败]: 红外感应CRC错误\n");
    } else {
        printf("  => [❌ 解析失败]: 红外感应协议格式错误\n");
    }
}

// 处理雷达的响应数据
void infrared_process_radar_data(const uint8_t *resp, size_t resp_len) {
    int state = 0;
    int rc = parse_infrared_radar_data(resp, resp_len, &state);
    if (rc == 0) {
        s_radar_state = state;
        pthread_mutex_lock(&g_sensor_data.lock);
        g_sensor_data.radar_state = state;
        pthread_mutex_unlock(&g_sensor_data.lock);
        // 把结果交由综合判断控制打印
        print_combined_detect_status();
    } else if (rc == -2) {
        printf("  => [❌ 解析失败]: 雷达感应CRC错误\n");
    } else {
        printf("  => [❌ 解析失败]: 雷达感应协议格式错误\n");
    }
}