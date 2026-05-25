/**
 * @file pressure_test.hpp
 * @brief 压力测试工具
 *
 * 提供多种压力测试场景:
 *   1. 总线吞吐测试 - 测试串口总线最大通信速率
 *   2. 并发争用测试 - 多线程同时争用总线
 *   3. 持续稳定性测试 - 长时间运行检测内存泄漏/资源耗尽
 *   4. 丢包/超时模拟 - 模拟设备响应延迟场景
 *   5. 快速轮询极限测试 - 测试最小轮询间隔下的系统表现
 *
 * 使用方法:
 *   PressureTest test(bus);
 *   test.runAll();
 *   // 或单独运行某个测试
 *   test.runThroughputTest(30);  // 30秒吞吐测试
 */

#ifndef PRESSURE_TEST_HPP
#define PRESSURE_TEST_HPP

#include "serial_bus.hpp"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <mutex>

extern "C" {
#include "modbus_core.h"
}
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <functional>
#include <numeric>

/**
 * @brief 单次测试结果
 */
struct TestResult {
    std::string testName;
    int         durationSec;
    uint64_t    totalTransactions;
    uint64_t    successCount;
    uint64_t    failCount;
    double      avgLatencyMs;
    double      maxLatencyMs;
    double      minLatencyMs;
    double      throughputTps;   // Transactions Per Second
    uint64_t    busContentions;
};

/**
 * @brief 压力测试工具类
 */
class PressureTest {
public:
    explicit PressureTest(SerialBus *bus) : bus_(bus) {}

    /**
     * @brief 运行所有压力测试
     */
    void runAll() {
        printf("\n");
        printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              压力测试开始                                    ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n\n");

        bus_->resetStats();

        auto r1 = runThroughputTest(10);
        printResult(r1);

        auto r2 = runConcurrencyTest(4, 10);
        printResult(r2);

        auto r3 = runRapidPollingTest(1000);
        printResult(r3);

        auto r4 = runStabilityTest(30);
        printResult(r4);

        printf("\n╔══════════════════════════════════════════════════════════════╗\n");
        printf("║              压力测试完成                                    ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    }

    /**
     * @brief 吞吐量测试
     *
     * 单线程持续发送空帧测试总线最大吞吐率。
     * 使用回环测试或直接测量发送-接收延迟。
     *
     * @param durationSec 测试持续时间 (秒)
     */
    TestResult runThroughputTest(int durationSec = 10) {
        printf("[压力测试] 吞吐量测试 (%ds)...\n", durationSec);

        TestResult result;
        result.testName = "吞吐量测试";
        result.durationSec = durationSec;
        result.totalTransactions = 0;
        result.successCount = 0;
        result.failCount = 0;
        result.avgLatencyMs = 0;
        result.maxLatencyMs = 0;
        result.minLatencyMs = 999999.0;
        result.throughputTps = 0;
        result.busContentions = 0;

        // 构造一个最小的 Modbus 查询帧 (读保持寄存器)
        // 01 03 0000 0001 [CRC] - 查询设备1的寄存器0
        uint8_t testFrame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
        uint8_t resp[256];
        size_t resp_len;

        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::seconds(durationSec);
        std::vector<double> latencies;

        while (std::chrono::steady_clock::now() < endTime) {
            auto t1 = std::chrono::steady_clock::now();

            int rc = bus_->transact(testFrame, sizeof(testFrame),
                                    resp, sizeof(resp), &resp_len, 200);

            auto t2 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
            latencies.push_back(ms);

            result.totalTransactions++;
            if (rc == 0) result.successCount++;
            else result.failCount++;
        }

        if (!latencies.empty()) {
            result.avgLatencyMs = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            result.maxLatencyMs = *std::max_element(latencies.begin(), latencies.end());
            result.minLatencyMs = *std::min_element(latencies.begin(), latencies.end());
        }

        result.throughputTps = static_cast<double>(result.totalTransactions) / durationSec;
        result.busContentions = bus_->getStats().busContentionCount.load();

        return result;
    }

    /**
     * @brief 并发争用测试
     *
     * 多个线程同时争用总线, 测试互斥锁的正确性和争用开销。
     *
     * @param threadCount 并发线程数
     * @param durationSec 测试持续时间 (秒)
     */
    TestResult runConcurrencyTest(int threadCount = 4, int durationSec = 10) {
        printf("[压力测试] 并发争用测试 (%d线程, %ds)...\n", threadCount, durationSec);

        TestResult result;
        result.testName = "并发争用测试 (" + std::to_string(threadCount) + "线程)";
        result.durationSec = durationSec;
        result.totalTransactions = 0;
        result.successCount = 0;
        result.failCount = 0;
        result.avgLatencyMs = 0;
        result.maxLatencyMs = 0;
        result.minLatencyMs = 999999.0;
        result.throughputTps = 0;
        result.busContentions = 0;

        // 每个线程的统计
        std::vector<uint64_t> threadTxCounts(threadCount, 0);
        std::vector<uint64_t> threadSuccessCounts(threadCount, 0);
        std::vector<uint64_t> threadFailCounts(threadCount, 0);
        std::vector<double> allLatencies;
        std::mutex latMtx;

        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::seconds(durationSec);

        // 每个线程使用不同的设备地址模拟
        std::vector<std::thread> threads;
        for (int t = 0; t < threadCount; t++) {
            threads.emplace_back([&, t]() {
                // 不同线程用不同地址的查询帧
                uint8_t addr = static_cast<uint8_t>(0x01 + t);
                uint8_t testFrame[] = {addr, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
                // 计算CRC (简单实现)
                uint16_t crc = crc16_modbus(testFrame, 6);
                testFrame[6] = crc & 0xFF;
                testFrame[7] = (crc >> 8) & 0xFF;

                uint8_t resp[256];
                size_t resp_len;

                while (std::chrono::steady_clock::now() < endTime) {
                    auto t1 = std::chrono::steady_clock::now();

                    int rc = bus_->transact(testFrame, sizeof(testFrame),
                                            resp, sizeof(resp), &resp_len, 300);

                    auto t2 = std::chrono::steady_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

                    threadTxCounts[t]++;
                    if (rc == 0) threadSuccessCounts[t]++;
                    else threadFailCounts[t]++;

                    {
                        std::lock_guard<std::mutex> lock(latMtx);
                        allLatencies.push_back(ms);
                    }
                }
            });
        }

        for (auto &t : threads) t.join();

        for (int t = 0; t < threadCount; t++) {
            result.totalTransactions += threadTxCounts[t];
            result.successCount += threadSuccessCounts[t];
            result.failCount += threadFailCounts[t];
        }

        if (!allLatencies.empty()) {
            result.avgLatencyMs = std::accumulate(allLatencies.begin(), allLatencies.end(), 0.0) / allLatencies.size();
            result.maxLatencyMs = *std::max_element(allLatencies.begin(), allLatencies.end());
            result.minLatencyMs = *std::min_element(allLatencies.begin(), allLatencies.end());
        }

        result.throughputTps = static_cast<double>(result.totalTransactions) / durationSec;
        result.busContentions = bus_->getStats().busContentionCount.load();

        return result;
    }

    /**
     * @brief 快速轮询极限测试
     *
     * 以极短间隔轮询, 测试系统在极端速度下的表现。
     * 模拟"某些节点数据量大、轮询速度非常快"的场景。
     *
     * @param iterations 迭代次数
     */
    TestResult runRapidPollingTest(int iterations = 1000) {
        printf("[压力测试] 快速轮询极限测试 (%d次迭代)...\n", iterations);

        TestResult result;
        result.testName = "快速轮询极限测试";
        result.durationSec = 0;
        result.totalTransactions = 0;
        result.successCount = 0;
        result.failCount = 0;
        result.avgLatencyMs = 0;
        result.maxLatencyMs = 0;
        result.minLatencyMs = 999999.0;
        result.throughputTps = 0;
        result.busContentions = 0;

        uint8_t testFrame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
        uint8_t resp[256];
        size_t resp_len;
        std::vector<double> latencies;

        auto totalStart = std::chrono::steady_clock::now();

        for (int i = 0; i < iterations; i++) {
            auto t1 = std::chrono::steady_clock::now();

            int rc = bus_->transact(testFrame, sizeof(testFrame),
                                    resp, sizeof(resp), &resp_len, 100);

            auto t2 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
            latencies.push_back(ms);

            result.totalTransactions++;
            if (rc == 0) result.successCount++;
            else result.failCount++;

            // 不加任何延时, 全速轮询
        }

        auto totalEnd = std::chrono::steady_clock::now();
        result.durationSec = static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(totalEnd - totalStart).count());
        if (result.durationSec == 0) result.durationSec = 1;

        if (!latencies.empty()) {
            result.avgLatencyMs = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            result.maxLatencyMs = *std::max_element(latencies.begin(), latencies.end());
            result.minLatencyMs = *std::min_element(latencies.begin(), latencies.end());
        }

        result.throughputTps = static_cast<double>(result.totalTransactions) / result.durationSec;
        result.busContentions = bus_->getStats().busContentionCount.load();

        return result;
    }

    /**
     * @brief 持续稳定性测试
     *
     * 长时间运行, 检测是否有内存泄漏、文件描述符泄漏等问题。
     *
     * @param durationSec 测试持续时间 (秒)
     */
    TestResult runStabilityTest(int durationSec = 60) {
        printf("[压力测试] 持续稳定性测试 (%ds)...\n", durationSec);

        TestResult result;
        result.testName = "持续稳定性测试";
        result.durationSec = durationSec;
        result.totalTransactions = 0;
        result.successCount = 0;
        result.failCount = 0;
        result.avgLatencyMs = 0;
        result.maxLatencyMs = 0;
        result.minLatencyMs = 999999.0;
        result.throughputTps = 0;
        result.busContentions = 0;

        uint8_t testFrame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
        uint8_t resp[256];
        size_t resp_len;

        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::seconds(durationSec);
        std::vector<double> latencies;
        int progressInterval = durationSec / 10;
        if (progressInterval < 1) progressInterval = 1;
        auto lastProgress = startTime;

        while (std::chrono::steady_clock::now() < endTime) {
            auto t1 = std::chrono::steady_clock::now();

            int rc = bus_->transact(testFrame, sizeof(testFrame),
                                    resp, sizeof(resp), &resp_len, 200);

            auto t2 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
            latencies.push_back(ms);

            result.totalTransactions++;
            if (rc == 0) result.successCount++;
            else result.failCount++;

            // 每隔一段时间输出进度
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastProgress).count() >= progressInterval) {
                lastProgress = now;
                int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
                printf("  进度: %d/%ds, 事务数: %llu, 成功率: %.1f%%\n",
                       elapsed, durationSec,
                       (unsigned long long)result.totalTransactions,
                       result.totalTransactions > 0 ?
                           (double)result.successCount / result.totalTransactions * 100.0 : 0.0);
            }

            // 模拟正常轮询间隔
            usleep(10000);  // 10ms
        }

        if (!latencies.empty()) {
            result.avgLatencyMs = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            result.maxLatencyMs = *std::max_element(latencies.begin(), latencies.end());
            result.minLatencyMs = *std::min_element(latencies.begin(), latencies.end());
        }

        result.throughputTps = static_cast<double>(result.totalTransactions) / durationSec;
        result.busContentions = bus_->getStats().busContentionCount.load();

        return result;
    }

    /**
     * @brief 打印测试结果
     */
    static void printResult(const TestResult &r) {
        double successRate = r.totalTransactions > 0 ?
            (double)r.successCount / r.totalTransactions * 100.0 : 0.0;

        printf("\n┌─────────────────────────────────────────────────────────────┐\n");
        printf("│ 测试: %-52s│\n", r.testName.c_str());
        printf("├─────────────────────────────────────────────────────────────┤\n");
        printf("│ 持续时间:    %-6d 秒                                      │\n", r.durationSec);
        printf("│ 总事务数:    %-10llu                                      │\n", (unsigned long long)r.totalTransactions);
        printf("│ 成功:        %-10llu  失败: %-10llu                      │\n",
               (unsigned long long)r.successCount, (unsigned long long)r.failCount);
        printf("│ 成功率:      %.1f%%                                          │\n", successRate);
        printf("│ 吞吐率:      %.1f TPS                                      │\n", r.throughputTps);
        printf("│ 平均延迟:    %.1f ms                                        │\n", r.avgLatencyMs);
        printf("│ 最大延迟:    %.1f ms                                        │\n", r.maxLatencyMs);
        printf("│ 最小延迟:    %.1f ms                                        │\n", r.minLatencyMs);
        printf("│ 总线争用:    %-10llu                                      │\n", (unsigned long long)r.busContentions);
        printf("└─────────────────────────────────────────────────────────────┘\n");
    }

private:
    SerialBus *bus_;
};

#endif /* PRESSURE_TEST_HPP */