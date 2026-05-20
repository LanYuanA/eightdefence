/**
 * @file main.cpp
 * @brief 系统主入口 (C++)
 *
 * 使用表驱动法，统一管理所有设备轮询任务。
 * 替代原 main.c，使用C++面向对象的设备抽象层。
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

extern "C" {
#include "modbus_core.h"
#include "web_server.h"
}

#include "service/modbus_service.hpp"
#include "devices/dev_cloud_sensors.hpp"
#include "devices/dev_smoke.hpp"
#include "devices/dev_water.hpp"
#include "devices/dev_infrared.hpp"
#include "devices/dev_light.hpp"
#include "devices/dev_humidifier.hpp"
#include "core/global_devices.hpp"

/* ============================================================
 * 设备实例
 * ============================================================ */
/* 云测仪 - 拆分为8个独立传感器 */
DevCloudPm25        dev_pm25;
DevCloudPm10        dev_pm10;
DevCloudHumidity    dev_humidity;
DevCloudTemperature dev_temperature;
DevCloudTvoc        dev_tvoc;
DevCloudCh2o        dev_ch2o;
DevCloudO3          dev_o3;
DevCloudCo2         dev_co2;

/* 其他设备 */
DevSmoke            dev_smoke;
DevWater            dev_water;
DevInfrared         dev_infrared;
DevLight            dev_light;
DevHumidifier       dev_humidifier;
DevAirConditioner   dev_ac;
DevAirPurifier      dev_purifier;
DevAlarmDevice      dev_alarm;

ModbusService*      g_modbus = nullptr;

/* ============================================================
 * 初始化所有设备
 * ============================================================ */
static void init_all_devices() {
    dev_pm25.init();
    dev_pm10.init();
    dev_humidity.init();
    dev_temperature.init();
    dev_tvoc.init();
    dev_ch2o.init();
    dev_o3.init();
    dev_co2.init();
    dev_smoke.init();
    dev_water.init();
    dev_infrared.init();
    dev_light.init();
    dev_humidifier.init();
    dev_ac.init();
    dev_purifier.init();
    dev_alarm.init();
}

int main(int argc, char *argv[]) {
    /* 初始化传感器数据缓冲并启动 Web 服务器 */
    init_all_devices();
    pthread_t web_tid;
    pthread_create(&web_tid, NULL, start_web_server, NULL);

    /* 创建Modbus服务 */
    const char *device = "/dev/ttyS9";
    if (argc >= 2) device = argv[1];
    ModbusService modbus(device);
    g_modbus = &modbus;

    uint8_t response[512];
    size_t  response_len = 0;

    /* ============================================================
     * 收集所有设备的任务
     * ============================================================ */
    std::vector<DeviceTask> all_tasks;

    /* 按原main.c顺序排列:
     * 云测仪(PM2.5/PM10/湿度/温度/TVOC/甲醛/臭氧/CO2)
     * -> 烟雾 -> 恒湿机 -> 报警器 -> 水浸 -> 红外 -> 净化机 -> 光感 */
    {
        auto t = dev_pm25.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_pm10.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_humidity.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_temperature.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_tvoc.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_ch2o.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_o3.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_co2.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_smoke.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_humidifier.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_alarm.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_water.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_infrared.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    /* 空调集中控制器: 仅控制无轮询 */
    {
        auto t = dev_purifier.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_light.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }

    size_t num_tasks = all_tasks.size();

    /* ============================================================
     * 主循环: 轮询所有设备任务
     * ============================================================ */
    while (1) {
        for (size_t i = 0; i < num_tasks; i++) {
            printf("==== [%zu/%zu] %s ====\n", i + 1, num_tasks, all_tasks[i].description.c_str());

            int rc = all_tasks[i].readFunc(modbus, response, &response_len);

            if (rc == 0) {
                printf("RX SUCCESS: ");
                print_hex_bytes(response, response_len);
            } else {
                printf("RX FAILED : Error Code %d\n", rc);
            }

            all_tasks[i].processFunc(response, response_len, rc);
            printf("\n");
        }
        sleep(60);
    }

    return 0;
}