#ifndef DEV_SMOKE_H
#define DEV_SMOKE_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

typedef struct {
    int smoke_status;
    int smoke_temp_thresh;
    int online;
    int fail_count;
    pthread_mutex_t lock;
} DevSmokeData;

extern DevSmokeData g_dev_smoke_data;
void init_dev_smoke_data();


int smoke_read_alarm_status(const char* device, uint8_t *resp, size_t *resp_len);

// 新增设备级解析数据的对外接口
void smoke_process_alarm_data(const uint8_t *resp, size_t resp_len, int rc);

#endif
