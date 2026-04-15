#ifndef DEV_CLOUD_H
#define DEV_CLOUD_H

#include <stdint.h>
#include <stddef.h>

int cloud_read_pm25(const char* device, uint8_t *resp, size_t *resp_len);
int cloud_read_pm10(const char* device, uint8_t *resp, size_t *resp_len);
int cloud_read_humidity(const char* device, uint8_t *resp, size_t *resp_len);
int cloud_read_temperature(const char* device, uint8_t *resp, size_t *resp_len);
int cloud_read_tvoc(const char* device, uint8_t *resp, size_t *resp_len);
int cloud_read_ch2o(const char* device, uint8_t *resp, size_t *resp_len);
int cloud_read_o3(const char* device, uint8_t *resp, size_t *resp_len);
int cloud_read_co2(const char* device, uint8_t *resp, size_t *resp_len);

// 新增数据处理机制：枚举类型
typedef enum {
    CLOUD_TYPE_PM25,
    CLOUD_TYPE_PM10,
    CLOUD_TYPE_HUMIDITY,
    CLOUD_TYPE_TEMPERATURE,
    CLOUD_TYPE_TVOC,
    CLOUD_TYPE_CH2O,
    CLOUD_TYPE_O3,
    CLOUD_TYPE_CO2
} cloud_sensor_type_t;

// 每个传感器类型的独立包裹函数，供 main.c 回调注册使用
void cloud_process_pm25(const uint8_t *resp, size_t resp_len);
void cloud_process_pm10(const uint8_t *resp, size_t resp_len);
void cloud_process_humidity(const uint8_t *resp, size_t resp_len);
void cloud_process_temperature(const uint8_t *resp, size_t resp_len);
void cloud_process_tvoc(const uint8_t *resp, size_t resp_len);
void cloud_process_ch2o(const uint8_t *resp, size_t resp_len);
void cloud_process_o3(const uint8_t *resp, size_t resp_len);
void cloud_process_co2(const uint8_t *resp, size_t resp_len);

#endif
