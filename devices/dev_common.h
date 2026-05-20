#ifndef DEV_COMMON_H
#define DEV_COMMON_H

/**
 * @file dev_common.h
 * @brief 设备抽象层 - 公共工具
 * 
 * 提供设备抽象层共用的数据结构和工具函数，包括：
 * - 通用设备状态管理（在线/离线/失败计数）
 * - 环境数据公共结构体（温湿度、PM2.5等）
 */

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 通用设备运行状态
 */
typedef struct {
    int online;          /* 1=在线, 0=离线 */
    int fail_count;      /* 连续通信失败次数 */
    pthread_mutex_t lock;
} DeviceStatus;

/**
 * @brief 初始化设备状态
 */
void dev_status_init(DeviceStatus *st);

/**
 * @brief 通信成功时更新状态
 */
void dev_status_on_success(DeviceStatus *st);

/**
 * @brief 通信失败时更新状态，超过阈值则标记离线
 * @param threshold 离线判定阈值（连续失败次数）
 * @return 1=刚变为离线, 0=未变化
 */
int dev_status_on_failure(DeviceStatus *st, int threshold);

/**
 * @brief 读取当前在线状态（线程安全）
 */
int dev_status_is_online(DeviceStatus *st);

/**
 * @brief 公共环境数据结构（多设备共用）
 */
typedef struct {
    int pm25;            /* PM2.5 浓度 (ug/m3) */
    int pm10;            /* PM10 浓度 (ug/m3) */
    int humidity;        /* 湿度 (%RH * 10) */
    int temperature;     /* 温度 (℃ * 10) */
    int tvoc;            /* TVOC 浓度 (ppb) */
    int ch2o;            /* 甲醛浓度 (mg/m3 * 1000) */
    int co2;             /* CO2 浓度 (ppm) */
    int o3;              /* 臭氧浓度 (ppb) */
    pthread_mutex_t lock;
} EnvData;

/**
 * @brief 初始化环境数据
 */
void env_data_init(EnvData *data);

#ifdef __cplusplus
}
#endif

#endif /* DEV_COMMON_H */