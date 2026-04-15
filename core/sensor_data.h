#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <pthread.h>

// 全局传感器状态缓存结构体
typedef struct {
    float temp;
    float hum;
    int pm25;
    int pm10;
    int tvoc;
    int ch2o;
    int o3;
    int co2;
    int smoke_status;
    int smoke_temp_thresh;
    int water_status;
    int ir_state;
    int radar_state;
    int lux;
    pthread_mutex_t lock;
} SensorData;

extern SensorData g_sensor_data;

void init_sensor_data();

#endif // SENSOR_DATA_H