#include "dev_cloud.h"
#include "device_config.h"
#include "modbus_core.h"

#include <stdio.h>
#include <string.h>

DevCloudData g_dev_cloud_data;

void init_dev_cloud_data() {
    memset(&g_dev_cloud_data, 0, sizeof(DevCloudData));
    pthread_mutex_init(&g_dev_cloud_data.lock, NULL);
}


int cloud_read_pm25(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_PM25, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}

int cloud_read_pm10(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_PM10, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}

int cloud_read_humidity(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_HUMIDITY, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}

int cloud_read_temperature(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_TEMP, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}

int cloud_read_tvoc(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_TVOC, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}

int cloud_read_ch2o(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_CH2O, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}

int cloud_read_o3(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_O3, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}

int cloud_read_co2(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_CLOUD_ADDR, REG_READ, REG_CLOUD_CO2, REG_CLOUD_DATA, resp, 512, resp_len, 300);
}



static void cloud_process_data_internal(const uint8_t *resp, size_t resp_len, cloud_sensor_type_t sensor_type, int rc) {
    pthread_mutex_lock(&g_dev_cloud_data.lock);
    if (rc != 0) {
        g_dev_cloud_data.fail_count++;
        if (g_dev_cloud_data.fail_count >= 3) {
            g_dev_cloud_data.online = 0;
            printf("  => [⚠️ 设备离线]: 云测仪连续3次未读到数据\n");
        }
        pthread_mutex_unlock(&g_dev_cloud_data.lock);
        return;
    }
    g_dev_cloud_data.fail_count = 0;
    g_dev_cloud_data.online = 1;
    pthread_mutex_unlock(&g_dev_cloud_data.lock);

    if (resp_len >= 7 && resp[0] == DEV_CLOUD_ADDR && resp[1] == 0x03) {
        
        // 1. 进行 CRC16 校验
        uint16_t expected_crc = crc16_modbus(resp, resp_len - 2);
        uint16_t actual_crc = resp[resp_len - 2] | (resp[resp_len - 1] << 8);
        if (expected_crc != actual_crc) {
             printf("  => [❌ 解析失败]: 云测仪数据 CRC 校验报错\n");
             return;
        }

        // 2. 取出回包数据(1个寄存器=2字节)
        uint16_t data = (resp[3] << 8) | resp[4];

        // 3. 根据原 Java 方案分类进行：模型赋值 与 二次处理
        pthread_mutex_lock(&g_dev_cloud_data.lock);
        switch(sensor_type) {
            case CLOUD_TYPE_TEMPERATURE: {
                // 温度读回的整型通常放大了 10 倍
                float temp_actual = data / 10.0f;
                g_dev_cloud_data.temp = temp_actual;
                printf("  => [🌡️ 模拟赋值 IndoorModel.setTemperature]: %.1f ℃\n", temp_actual);
                printf("  => [📊 模拟入库 ChartDataUtils.addTemperatureData]: %.1f ℃\n", temp_actual);
                break;
            }
            case CLOUD_TYPE_HUMIDITY: {
                // 湿度同理除了 10
                float hum_actual = data / 10.0f;
                g_dev_cloud_data.hum = hum_actual;
                printf("  => [💧 模拟赋值 IndoorModel.setHumidity]: %.1f %%RH\n", hum_actual);
                printf("  => [📊 模拟入库 ChartDataUtils.addHumidityData]: %.1f %%RH\n", hum_actual);
                break;
            }
            case CLOUD_TYPE_PM25:
                g_dev_cloud_data.pm25 = data;
                printf("  => [🌬️ 模拟赋值 IndoorModel.setPm25]: %d μg/m³\n", data);
                break;
            case CLOUD_TYPE_PM10:
                g_dev_cloud_data.pm10 = data;
                printf("  => [🌬️ 模拟赋值 IndoorModel.setPm10]: %d μg/m³\n", data);
                break;
            case CLOUD_TYPE_TVOC:
                g_dev_cloud_data.tvoc = data;
                printf("  => [☣️ 模拟赋值 IndoorModel.setTvoc]: %d ppb\n", data);
                break;
            case CLOUD_TYPE_CH2O:
                g_dev_cloud_data.ch2o = data;
                printf("  => [🧪 模拟赋值 IndoorModel.setCh2o (甲醛)]: %d ppm\n", data);
                break;
            case CLOUD_TYPE_O3:
                g_dev_cloud_data.o3 = data;
                printf("  => [☁️ 模拟赋值 IndoorModel.setO3 (臭氧)]: %d ppm\n", data);
                break;
            case CLOUD_TYPE_CO2:
                g_dev_cloud_data.co2 = data;
                printf("  => [💨 模拟赋值 IndoorModel.setCo2 (二氧化氮)]: %d ppm\n", data);
                break;
        }
        pthread_mutex_unlock(&g_dev_cloud_data.lock);
    } else {
        printf("  => [❌ 解析失败]: 数据包头或地址不匹配\n");
    }
}

// 暴露出匹配 main.c 单一对应参数表的回调接口
void cloud_process_pm25(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_PM25, rc); }
void cloud_process_pm10(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_PM10, rc); }
void cloud_process_humidity(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_HUMIDITY, rc); }
void cloud_process_temperature(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_TEMPERATURE, rc); }
void cloud_process_tvoc(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_TVOC, rc); }
void cloud_process_ch2o(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_CH2O, rc); }
void cloud_process_o3(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_O3, rc); }
void cloud_process_co2(const uint8_t *resp, size_t resp_len, int rc) { cloud_process_data_internal(resp, resp_len, CLOUD_TYPE_CO2, rc); }
