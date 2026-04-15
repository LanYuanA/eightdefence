#ifndef DEV_LIGHT_H
#define DEV_LIGHT_H

#include <stdint.h>
#include <stddef.h>

int light_read_illuminance(const char* device, uint8_t *resp, size_t *resp_len);

// 声明对外暴露的光照度数据处理模块
void light_process_illuminance_data(const uint8_t *resp, size_t resp_len);

#endif // DEV_LIGHT_H