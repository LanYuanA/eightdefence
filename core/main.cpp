/**
 * @file main.cpp
 * @brief 系统主入口 (C++) - v2.0 多线程架构
 *
 * 使用 PollingManager 管理多线程设备轮询。
 * 每个设备任务在独立线程中运行，通过共享的 SerialBus
 * (线程安全互斥锁) 访问 RS-485 总线。
 * 支持:
 *   - 每个设备独立的轮询间隔
 *   - 线程安全的总线访问
 *   - 非阻塞式轮询 (慢设备不阻塞快设备)
 *   - 运行时统计与压力测试
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <ctime>
#include <cerrno>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>

extern "C" {
#include "modbus_core.h"
#include "web_server.h"
}

#include "serial_bus.hpp"
#include "polling_manager.hpp"
#include "pressure_test.hpp"
#include "command_queue.hpp"
#include "logger.hpp"
#include "service/modbus_service.hpp"
#include "devices/dev_cloud_sensors.hpp"
#include "devices/dev_smoke.hpp"
#include "devices/dev_water.hpp"
#include "devices/dev_infrared.hpp"
#include "devices/dev_light.hpp"
#include "devices/dev_humidifier.hpp"
#include "devices/dev_air_conditioner.hpp"
#include "devices/dev_air_purifier.hpp"
#include "devices/dev_alarm_device.hpp"
#include "global_devices.hpp"
#include "application/app_manager.hpp"
#include "application/apps/security/app_security.hpp"
#include "service/atomic/svc_sound_light_alarm.hpp"
#include "service/atomic/svc_drainage.hpp"
#include "service/atomic/svc_temp_humidity_control.hpp"
#include "service/atomic/svc_gas_response.hpp"
#include "service/atomic/svc_command_center.hpp"

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
SerialBus*          g_serial_bus = nullptr;
CommandQueue*       g_cmd_queue = nullptr;
volatile bool       g_web_running = true;

// 信号通知管道 (self-pipe trick)
static int g_signal_pipe[2] = {-1, -1};

static void signal_handler(int sig) {
    (void)sig;
    g_web_running = false;
    // 写一个字节到管道, 唤醒 select()/read()
    char c = 'X';
    write(g_signal_pipe[1], &c, 1);
    const char msg[] = "[SIGNAL] caught\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
}

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

/* ============================================================
 * 帮助信息
 * ============================================================ */
static void print_usage(const char *prog) {
    printf("用法: %s [选项]\n", prog);
    printf("选项:\n");
    printf("  -d <device>    串口设备路径 (默认: /dev/ttyS9)\n");
    printf("  -b <baud>      波特率 (默认: 9600)\n");
    printf("  -w <threads>   工作线程数 (默认: 4)\n");
    printf("  -t <seconds>   压力测试持续时间 (秒, 0=不测试, 默认: 0)\n");
    printf("  -c <count>     压力测试并发线程数 (默认: 4)\n");
    printf("  -h             显示此帮助\n");
}

int main(int argc, char *argv[]) {
    /* 解析命令行参数 */
    const char *device = "/dev/ttyS9";
    int baud = 9600;
    int worker_threads = 4;
    int stress_duration = 0;
    int stress_concurrency = 4;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            device = argv[++i];
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            baud = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            worker_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            stress_duration = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            stress_concurrency = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    /* 创建信号通知管道 (self-pipe trick) */
    if (pipe(g_signal_pipe) != 0) {
        fprintf(stderr, "创建信号管道失败!\n");
        return 1;
    }
    // 设置读端为非阻塞
    fcntl(g_signal_pipe[0], F_SETFL, O_NONBLOCK);

    /* 注册信号处理 (使用 sigaction 确保不设置 SA_RESTART) */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // 不设置 SA_RESTART, 让 select/nanosleep 被中断
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* 初始化日志系统 */
    Logger::instance().init("./logs", 50, LogLevel::INFO);
    LOG_INFO("=== 系统启动 ===");
    LOG_INFO("串口: %s, 波特率: %d, 工作线程: %d", device, baud, worker_threads);

    /* 初始化传感器数据缓冲 */
    init_all_devices();
    LOG_INFO("所有设备已初始化");

    /* 创建原子服务 */
    SvcSoundLightAlarm     svcSoundLight;
    SvcDrainage            svcDrainage;
    SvcTempHumidityControl svcTempHumid;
    SvcGasResponse         svcGasResp;
    SvcCommandCenter       svcCmdCenter;

    /* 注册应用到 AppManager, 注入服务 */
    auto& appMgr = AppManager::instance();
    auto securityApp = std::make_shared<AppSecurity>();
    securityApp->setServices(&svcSoundLight, &svcDrainage, &svcTempHumid, &svcGasResp, &svcCmdCenter);
    appMgr.registerApp(securityApp);
    appMgr.initAll();
    appMgr.startAll();
    LOG_INFO("应用管理器已启动, 注册了 %zu 个应用", appMgr.getAllApps().size());

    /* 启动 Web 服务器 */
    pthread_t web_tid;
    pthread_create(&web_tid, NULL, start_web_server, NULL);
    LOG_INFO("Web 服务器已启动");

    /* ============================================================
     * 创建线程安全的串口总线
     * ============================================================ */
    LOG_INFO("初始化串口总线: %s @ %d baud", device, baud);
    SerialBus serialBus(device, baud);
    int rc = serialBus.open();
    if (rc != 0) {
        LOG_ERROR("串口打开失败! 错误码: %d", rc);
        return 1;
    }
    g_serial_bus = &serialBus;

    /* 创建线程安全的 ModbusService */
    ModbusService modbusService(&serialBus);
    g_modbus = &modbusService;

    /* 创建并启动命令队列 */
    CommandQueue cmdQueue(&serialBus, &modbusService, 1);
    g_cmd_queue = &cmdQueue;
    cmdQueue.start();

    LOG_INFO("串口总线已就绪, 工作线程数: %d", worker_threads);

    /* ============================================================
     * 压力测试模式
     * ============================================================ */
    if (stress_duration > 0) {
        LOG_INFO("=== 压力测试模式 === 持续时间: %d 秒, 并发线程: %d",
             stress_duration, stress_concurrency);

        PressureTest test(&serialBus);

        printf("\n--- 吞吐量测试 (%ds) ---\n", stress_duration);
        auto r1 = test.runThroughputTest(stress_duration);
        PressureTest::printResult(r1);

        printf("\n--- 并发争用测试 (%d线程, %ds) ---\n", stress_concurrency, stress_duration);
        auto r2 = test.runConcurrencyTest(stress_concurrency, stress_duration);
        PressureTest::printResult(r2);

        printf("\n--- 快速轮询极限测试 ---\n");
        auto r3 = test.runRapidPollingTest(1000);
        PressureTest::printResult(r3);

        printf("\n--- 持续稳定性测试 (%ds) ---\n", stress_duration);
        auto r4 = test.runStabilityTest(stress_duration);
        PressureTest::printResult(r4);

    LOG_INFO("压力测试完成");
        return 0;
    }

    /* ============================================================
     * 收集所有设备的任务
     * ============================================================ */
    std::vector<DeviceTask> all_tasks;

    /* 按设备分组, 同一设备的多个寄存器读取合并为一组 */
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
    {
        auto t = dev_purifier.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }
    {
        auto t = dev_light.getTasks();
        all_tasks.insert(all_tasks.end(), t.begin(), t.end());
    }

    size_t num_tasks = all_tasks.size();
    LOG_INFO("共 %zu 个轮询任务:", num_tasks);

    for (size_t i = 0; i < num_tasks; i++) {
        LOG_INFO("  [%zu] %s (间隔: %u ms)",
                 i + 1,
                 all_tasks[i].description.c_str(),
                 all_tasks[i].pollIntervalMs);
    }

    /* ============================================================
     * 启动多线程轮询管理器
     * ============================================================ */
    PollingConfig pollCfg;
    pollCfg.maxConcurrentGroups = worker_threads;

    PollingManager poller(&serialBus, &modbusService, pollCfg);
    poller.addTasks(all_tasks);
    poller.start();

    printf("[MAIN] poller.start() 已返回, 进入主循环\n"); fflush(stdout);
    LOG_INFO("多线程轮询已启动, 按 Ctrl+C 退出");

    /* ============================================================
     * 主循环: 定期打印统计信息
     * (使用 select 监听信号管道, 10秒超时打印统计)
     * ============================================================ */
    int statsCounter = 0;
    while (g_web_running) {
        // select 等待信号管道可读, 超时10秒
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_signal_pipe[0], &rfds);
        struct timeval tv = {10, 0};
        int sel_rc = select(g_signal_pipe[0] + 1, &rfds, NULL, NULL, &tv);
        // Drain the signal pipe if readable
        if (sel_rc > 0 && FD_ISSET(g_signal_pipe[0], &rfds)) {
            char buf[16];
            while (read(g_signal_pipe[0], buf, sizeof(buf)) > 0) {}
        }
        if (!g_web_running) break;

        /* 每30秒打印一次统计 */
        if (++statsCounter < 3) continue;
        statsCounter = 0;

        /* 打印总线统计 */
        const BusStats &stats = serialBus.getStats();
        LOG_INFO("======== 总线统计 ========");
        LOG_INFO("  总事务数:     %lu", (unsigned long)stats.totalTransactions.load());
        LOG_INFO("  总错误数:     %lu", (unsigned long)stats.totalErrors.load());
        LOG_INFO("  发送字节数:   %lu", (unsigned long)stats.totalBytesSent.load());
        LOG_INFO("  接收字节数:   %lu", (unsigned long)stats.totalBytesRecv.load());
        LOG_INFO("  平均延迟:     %.2f ms", stats.avgLatencyMs.load());
        LOG_INFO("  最大延迟:     %.2f ms", stats.maxLatencyMs.load());
        LOG_INFO("  总线争用次数: %lu", (unsigned long)stats.busContentionCount.load());

        /* 打印轮询统计 */
        auto groupStats = poller.getGroupStatsSnapshot();
        uint64_t totalOk = 0, totalFail = 0;
        for (auto &kv : groupStats) {
            totalOk += kv.second.successCount;
            totalFail += kv.second.failCount;
        }
        LOG_INFO("======== 轮询统计 ========");
        LOG_INFO("  任务组数:     %zu", groupStats.size());
        LOG_INFO("  成功事务:     %lu", (unsigned long)totalOk);
        LOG_INFO("  失败事务:     %lu", (unsigned long)totalFail);
        if (totalOk + totalFail > 0) {
            LOG_INFO("  成功率:       %.1f%%",
                     100.0 * totalOk / (totalOk + totalFail));
        }
        /* 命令队列统计 */
        const auto &cmdStats = cmdQueue.getStats();
        LOG_INFO("======== 命令队列统计 ========");
        LOG_INFO("  已提交: %lu  已执行: %lu  成功: %lu  失败: %lu",
                 (unsigned long)cmdStats.totalSubmitted.load(),
                 (unsigned long)cmdStats.totalExecuted.load(),
                 (unsigned long)cmdStats.totalSuccess.load(),
                 (unsigned long)cmdStats.totalFailed.load());
        LOG_INFO("  队列大小: %lu  平均耗时: %.1fms",
                 (unsigned long)cmdStats.currentQueueSize.load(),
                 cmdStats.avgExecTimeMs.load());
    }

    /* ============================================================
     * 清理退出
     * ============================================================ */
    LOG_INFO("正在停止 Web 服务器...");
    pthread_join(web_tid, NULL);

    LOG_INFO("正在停止所有应用...");
    appMgr.stopAll();

    LOG_INFO("正在停止命令队列...");
    cmdQueue.stop();

    // 关闭串口 (写 shutdown_pipe 唤醒阻塞在 select() 上的轮询线程)
    LOG_INFO("正在关闭串口...");
    serialBus.close();

    LOG_INFO("正在停止轮询...");
    poller.stop();

    /* 打印最终统计 */
    const BusStats &finalStats = serialBus.getStats();
    LOG_INFO("======== 最终统计 ========");
    LOG_INFO("  总事务数:     %lu", (unsigned long)finalStats.totalTransactions.load());
    LOG_INFO("  总错误数:     %lu", (unsigned long)finalStats.totalErrors.load());
    LOG_INFO("  成功率:       %.1f%%",
             finalStats.totalTransactions > 0
                 ? 100.0 * (finalStats.totalTransactions - finalStats.totalErrors) / finalStats.totalTransactions
                 : 0.0);
    LOG_INFO("  平均延迟:     %.2f ms", finalStats.avgLatencyMs.load());
    LOG_INFO("  最大延迟:     %.2f ms", finalStats.maxLatencyMs.load());
    LOG_INFO("=== 系统退出 ===");

    Logger::instance().shutdown();

    return 0;
}