#ifndef DEV_SMOKE_H
#define DEV_SMOKE_H

#include <stdint.h>
#include <stddef.h>

int smoke_read_alarm_status(const char* device, uint8_t *resp, size_t *resp_len);

// 新增设备级解析数据的对外接口
void smoke_process_alarm_data(const uint8_t *resp, size_t resp_len);

#endif
