#ifndef DEV_WATER_H
#define DEV_WATER_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

typedef struct {
    int water_status;
    int online;
    int fail_count;
    pthread_mutex_t lock;
} DevWaterData;

extern DevWaterData g_dev_water_data;
void init_dev_water_data();


int water_read_status(const char* device, uint8_t *resp, size_t *resp_len);
int water_parse_data(const uint8_t *resp, size_t resp_len, int *is_immersed);

// 对外暴露的业务处理总接口
void water_process_data(const uint8_t *resp, size_t resp_len, int rc);

#endif // DEV_WATER_H