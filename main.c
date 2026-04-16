#include <stdio.h>
#include <pthread.h>
#include "modbus_core.h"
#include "web_server.h"
#include "dev_cloud.h"
#include "dev_smoke.h"
#include "dev_water.h"
#include "dev_infrared.h"
#include "dev_light.h"
#include <unistd.h>

// 1. 定义统一的设备API函数指针类型
typedef int (*device_api_func)(const char*, uint8_t*, size_t*);
typedef void (*device_parse_func)(const uint8_t*, size_t, int rc);

// 2. 定义一条任务的结构体
typedef struct {
    const char *description;      // 任务描述或名字
    device_api_func read_func;    // 取收函数
    device_parse_func parse_func; // 解析与业务处理函数 (可为主逻辑省去冗余代码)
} device_task_t;

//设备初始化
void init_dev(){
    init_dev_cloud_data();
    init_dev_smoke_data();
    init_dev_water_data();
    init_dev_infrared_data();
    init_dev_light_data();
}

int main(int argc, char *argv[]) {
    // 初始化传感器数据缓冲并启动 Web 服务器
    init_dev();
    pthread_t web_tid;
    //创建网络线程
    pthread_create(&web_tid, NULL, start_web_server, NULL);
    
    
    //未来有更多服务直接在此添加线程...

    const char *device = "/dev/ttyS9";
    uint8_t response[512];
    size_t response_len = 0;
    int rc;

    if (argc >= 2) device = argv[1];

    //把所有的API指令全部注册到这个数组表里（表驱动法）
    device_task_t tasks[] = {
        {"Cloud Instrument - PM2.5", cloud_read_pm25, cloud_process_pm25},
        {"Cloud Instrument - PM10", cloud_read_pm10, cloud_process_pm10},
        {"Cloud Instrument - Humidity", cloud_read_humidity, cloud_process_humidity},
        {"Cloud Instrument - Temperature", cloud_read_temperature, cloud_process_temperature},
        {"Cloud Instrument - TVOC", cloud_read_tvoc, cloud_process_tvoc},
        {"Cloud Instrument - CH2O", cloud_read_ch2o, cloud_process_ch2o},
        {"Cloud Instrument - O3", cloud_read_o3, cloud_process_o3},
        {"Cloud Instrument - CO2", cloud_read_co2, cloud_process_co2},
        {"Smoke Alarm - Status", smoke_read_alarm_status, smoke_process_alarm_data},
        {"Water Leak - Status", water_read_status, water_process_data},
        {"Infrared Detector - Infrared State", infrared_read_state, infrared_process_ir_data},
        {"Infrared Detector - Radar State", radar_read_state, infrared_process_radar_data},
        {"Light Sensor - Illuminance", light_read_illuminance, light_process_illuminance_data},
        // TODO: 这里可以继续追加比如 {"Mouse Trap - Status", mouse_read_status, mouse_process_data},
    };

    size_t num_tasks = sizeof(tasks) / sizeof(tasks[0]);
    size_t i;

    // 4. 统一的执行流，一直循环轮询获取传感器数据
    while (1) {
        for (i = 0; i < num_tasks; i++) {
            printf("==== [%zu/%zu] %s ====\n", i + 1, num_tasks, tasks[i].description);
            
            // 调用对应的 API 函数
            rc = tasks[i].read_func(device, response, &response_len);
            
            if (rc == 0) {
                printf("RX SUCCESS: ");
                print_hex_bytes(response, response_len);
            } else {
                printf("RX FAILED : Error Code %d\n", rc);
            }
            if (tasks[i].parse_func != NULL) {
                tasks[i].parse_func(response, response_len, rc);
            }
            printf("\n");
        }
        sleep(60); // 睡眠 60 秒再次轮询，设置更长的时间避免上一次轮询未结束
    }

    return 0;
}
