#ifndef DEV_LIGHT_H
#define DEV_LIGHT_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

typedef struct {
    int lux;
    int online;
    int fail_count;
    pthread_mutex_t lock;
} DevLightData;

extern DevLightData g_dev_light_data;
void init_dev_light_data();


int light_read_illuminance(const char* device, uint8_t *resp, size_t *resp_len);

// 声明对外暴露的光照度数据处理模块
void light_process_illuminance_data(const uint8_t *resp, size_t resp_len, int rc);

#endif // DEV_LIGHT_H