#include <stdio.h>
#include <pthread.h>
#include "modbus_core.h"
#include "web_server.h"
#include "service/modbus_service.h"
#include "dev_cloud.h"
#include "dev_smoke.h"
#include "dev_water.h"
#include "dev_infrared.h"
#include "dev_light.h"
#include "dev_humidifier.h"
#include "dev_air_conditioner.h"
#include "dev_air_purifier.h"
#include "dev_alarm_device.h"
#include <unistd.h>

/* ============================================================
 * 表驱动法: 统一设备API函数指针类型
 * ============================================================ */
typedef int  (*device_api_func)(ModbusContext *ctx, uint8_t *resp, size_t *resp_len);
typedef void (*device_parse_func)(const uint8_t *resp, size_t resp_len, int rc);

typedef struct {
    const char       *description;  /* 任务描述 */
    device_api_func   read_func;    /* 读取函数 */
    device_parse_func parse_func;   /* 解析处理函数 */
} device_task_t;

/* ============================================================
 * 设备初始化
 * ============================================================ */
void init_dev() {
    init_dev_cloud_data();
    init_dev_smoke_data();
    init_dev_water_data();
    init_dev_infrared_data();
    init_dev_light_data();
    init_dev_humidifier_data();
    init_dev_ac_data();
    init_dev_purifier_data();
    init_dev_alarm_data();
}

int main(int argc, char *argv[]) {
    /* 初始化传感器数据缓冲并启动 Web 服务器 */
    init_dev();
    pthread_t web_tid;
    pthread_create(&web_tid, NULL, start_web_server, NULL);

    /* 创建Modbus上下文 */
    const char *device = "/dev/ttyS9";
    if (argc >= 2) device = argv[1];
    ModbusContext ctx = createContext(device);

    uint8_t response[512];
    size_t response_len = 0;
    int rc;

    /* ============================================================
     * 所有API指令注册到表驱动数组 (ntf* / ntf*Proc)
     * ============================================================ */
    device_task_t tasks[] = {
        /* --- 01. 室内空气质量变送器 (云测仪) --- */
        {"云测仪 - PM2.5",         ntfPM25,            ntfPM25Proc},
        {"云测仪 - PM10",          ntfPM10,            ntfPM10Proc},
        {"云测仪 - 湿度",          ntfHmd,             ntfHmdProc},
        {"云测仪 - 温度",          ntfT,               ntfTProc},
        {"云测仪 - TVOC",          ntfTVOC,            ntfTVOCProc},
        {"云测仪 - CH2O",          ntfCH2O,            ntfCH2OProc},
        {"云测仪 - O3",            ntfO3,              ntfO3Proc},
        {"云测仪 - CO2",           ntfCO2,             ntfCO2Proc},
        /* --- 02. 烟雾报警器 --- */
        {"烟雾报警器 - 报警状态",   ntfSmoke,           ntfSmokeProc},
        /* --- 03. 恒湿净化一体机 --- */
        {"恒湿机 - 环境数据",       ntfHmdEnvData,      ntfHmdEnvDataProc},
        {"恒湿机 - 电源状态",       ntfHmdPowerSt,      ntfHmdPowerStProc},
        {"恒湿机 - 故障状态",       ntfHmdFltSt,        ntfHmdFltStProc},
        /* --- 04. 报警装置 (驱鼠器) --- */
        {"报警装置 - 状态",         ntfAlarmState,      ntfAlarmStateProc},
        /* --- 05. 水浸传感器 --- */
        {"水浸传感器 - 状态",       ntfWater,           ntfWaterProc},
        /* --- 06. 红外探测器 --- */
        {"红外探测器 - 红外+雷达",   ntfIR,              ntfIRProc},
        /* --- 07. 空调集中控制器 (仅控制, 无轮询读取) --- */
        /* --- 08. 霉菌空气净化机 --- */
        {"净化机 - 环境数据",       ntfPurEnvData,      ntfPurEnvDataProc},
        {"净化机 - 运行模式",       ntfPurRunMode,      ntfPurRunModeProc},
        {"净化机 - 电源状态",       ntfPurPowerSt,      ntfPurPowerStProc},
        /* --- 09. 弱光检测传感器 (光感) --- */
        {"光感传感器 - 光照度",     ntfLight,            ntfLightProc},
    };

    size_t num_tasks = sizeof(tasks) / sizeof(tasks[0]);
    size_t i;

    /* 统一执行流: 循环轮询所有传感器 */
    while (1) {
        for (i = 0; i < num_tasks; i++) {
            printf("==== [%zu/%zu] %s ====\n", i + 1, num_tasks, tasks[i].description);

            rc = tasks[i].read_func(&ctx, response, &response_len);

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
        sleep(60);
    }

    return 0;
}