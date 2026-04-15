#include "dev_smoke.h"
#include "device_config.h"
#include "modbus_core.h"
#include "sensor_data.h"
#include <stdio.h>

int smoke_read_alarm_status(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_SMOKE_ADDR, 0x03, REG_SMOKE_ALARM, 0x000A, resp, 512, resp_len, 300);
}

void smoke_process_alarm_data(const uint8_t *resp, size_t resp_len) {
    // 根据通信协议，发送 70 03 0001 000A 会响应 20 个字节（0x14 = 20字节数据）
    // 假设回包: 70 03 14 0000 0070 0003 0037 0001 0000 0000 0000 0000 0000 254C 
    // - resp[0]: 0x70 (地址)
    // - resp[1]: 0x03 (功能码)
    // - resp[2]: 0x14 (后续数据总长，即 20 个字节，对应 10 个寄存器)
    // - resp[3~4]: 0x0001 报警状态 (我们读取的是第二寄存器,由于是从0001开始读取10个)
    
    // 我们读取是从 REG 0x0001 开始读取 0x000A(10)个寄存器.
    // 所以返回的这 20 个字节依次对应的是：
    // [0001]: resp[3], resp[4]   -> 报警状态 (0:正常, 1:报警, 2:防拆, 3:报警+防拆)
    // [0002]: resp[5], resp[6]   -> 设备地址 (这里是0x0070, 即十进制112/十六进制70)
    // [0003]: resp[7], resp[8]   -> 波特率 (0x0003 代表 9600)
    // [0004]: resp[9], resp[10]  -> 温度报警值 (0x0037, 换算十进制是 55)
    // [0005]: resp[11], resp[12] -> 探测器模式 (0x0001)
    
    if (resp_len >= 25 && resp[0] == DEV_SMOKE_ADDR && resp[1] == 0x03 && resp[2] == 0x14) {
        
        // 校验 CRC
        uint16_t expected_crc = crc16_modbus(resp, resp_len - 2);
        uint16_t actual_crc = resp[resp_len - 2] | (resp[resp_len - 1] << 8);
        if (expected_crc != actual_crc) {
             printf("  => [❌ 解析失败]: 烟雾传感器数据 CRC 校验报错\n");
             return;
        }
        
        // 解析: 报警状态 [第1寄存器, 在resp序列里的resp[3][4]]
        uint16_t alarm_status = (resp[3] << 8) | resp[4];
        // 解析: 报警温度阈值 [第4寄存器, 相对首寄存器偏移了3位，对应resp里的resp[3+3*2]=resp[9], resp[10]]
        uint16_t temp_thresh = (resp[9] << 8) | resp[10];

        pthread_mutex_lock(&g_sensor_data.lock);
        g_sensor_data.smoke_status = alarm_status;
        g_sensor_data.smoke_temp_thresh = temp_thresh;
        pthread_mutex_unlock(&g_sensor_data.lock);

        // 统一打印报警结果
        switch(alarm_status) {
            case 0:
                printf("  => [✅ 烟感状态]: 正常工作 (温度警报阀值:%d℃)\n", temp_thresh);
                break;
            case 1:
                printf("  => [🚨 烟感报警]: 检测到浓烟/高温火灾警报！\n");
                break;
            case 2:
                printf("  => [⚠️ 烟感异常]: 触发防拆卸警报！\n");
                break;
            case 3:
                printf("  => [🚨 致命警报]: 检测到火灾，且同时触发防拆警报！\n");
                break;
            default:
                printf("  => [⚠️ 未知状态]: 收到神秘代码 %d\n", alarm_status);
                break;
        }

    } else {
        printf("  => [❌ 解析失败]: 响应长度不足以容纳20字节或包头标识不符\n");
    }
}
