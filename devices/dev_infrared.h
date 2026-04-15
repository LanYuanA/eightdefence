#ifndef DEV_INFRARED_H
#define DEV_INFRARED_H

#include <stdint.h>
#include <stddef.h>

int infrared_read_state(const char* device, uint8_t *resp, size_t *resp_len);
int radar_read_state(const char* device, uint8_t *resp, size_t *resp_len);

// 新增对外暴露的模块独立数据处理句柄
void infrared_process_ir_data(const uint8_t *resp, size_t resp_len);
void infrared_process_radar_data(const uint8_t *resp, size_t resp_len);

#endif // DEV_INFRARED_H