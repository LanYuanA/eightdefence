/**
 * @file device_base.hpp
 * @brief 设备抽象基类 - 定义统一的设备接口
 *
 * 所有设备驱动类继承此基类，实现统一的：
 *   - 读取接口 (read)
 *   - 解析处理接口 (process)
 *   - 状态管理
 */

#ifndef DEVICE_BASE_HPP
#define DEVICE_BASE_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <mutex>
#include <functional>
#include "service/parse_service.hpp"
#include <vector>
#include "service/modbus_service.hpp"

/**
 * @brief 设备运行状态
 */
struct DeviceStatusCpp {
    int  online     = 0;   // 1=在线, 0=离线
    int  fail_count = 0;   // 连续通信失败次数
    std::mutex mtx;

    DeviceStatusCpp() = default;

    // 禁止拷贝（因为含 mutex）
    DeviceStatusCpp(const DeviceStatusCpp &) = delete;
    DeviceStatusCpp &operator=(const DeviceStatusCpp &) = delete;

    /**
     * @brief 重置状态（替代赋值操作）
     */
    void reset() {
        std::lock_guard<std::mutex> lock(mtx);
        online = 0;
        fail_count = 0;
    }

    void onSuccess() {
        std::lock_guard<std::mutex> lock(mtx);
        online = 1;
        fail_count = 0;
    }

    int onFailure(int threshold) {
        std::lock_guard<std::mutex> lock(mtx);
        fail_count++;
        if (fail_count >= threshold) {
            online = 0;
            return 1; // 刚变为离线
        }
        return 0;
    }

    bool isOnline() {
        std::lock_guard<std::mutex> lock(mtx);
        return online != 0;
    }

    void forceOffline() {
        std::lock_guard<std::mutex> lock(mtx);
        online = 0;
    }
};

/**
 * @brief 环境数据公共结构（线程安全）
 */
struct EnvDataCpp {
    int pm25        = 0;
    int pm10        = 0;
    int humidity    = 0;
    int temperature = 0;
    int tvoc        = 0;
    int ch2o        = 0;
    int co2         = 0;
    int o3          = 0;
    mutable std::mutex mtx;

    EnvDataCpp() = default;
    EnvDataCpp(const EnvDataCpp &other) {
        std::lock_guard<std::mutex> lock(other.mtx);
        pm25 = other.pm25; pm10 = other.pm10;
        humidity = other.humidity; temperature = other.temperature;
        tvoc = other.tvoc; ch2o = other.ch2o;
        co2 = other.co2; o3 = other.o3;
    }
    EnvDataCpp& operator=(const EnvDataCpp &other) {
        if (this != &other) {
            std::scoped_lock lock(mtx, other.mtx);
            pm25 = other.pm25; pm10 = other.pm10;
            humidity = other.humidity; temperature = other.temperature;
            tvoc = other.tvoc; ch2o = other.ch2o;
            co2 = other.co2; o3 = other.o3;
        }
        return *this;
    }
};

/**
 * @brief 设备轮询任务描述（使用std::function避免前向声明问题）
 */
struct DeviceTask {
    std::string description;   // 任务描述

    // 读取和处理函数放在前面, 保持与现有设备代码的兼容性
    std::function<int(ModbusService &svc, uint8_t *resp, size_t *resp_len)> readFunc;
    std::function<void(const uint8_t *resp, size_t resp_len, int rc)> processFunc;

    // 可配置参数 (带默认值, 可按设备自定义)
    int         pollIntervalMs = 5000;  // 轮询间隔(毫秒), 可针对不同设备自定义
    int         timeoutMs      = 1000;  // 单次通信超时(毫秒)
    uint8_t     devAddr        = 0;     // Modbus 设备地址 (用于日志/调试)
    int         priority       = 0;     // 优先级: 0=普通, 1=高优先级(安全类设备)
};

/**
 * @brief 设备抽象基类
 */
class DeviceBase {
public:
    virtual ~DeviceBase() = default;

    /**
     * @brief 初始化设备数据
     */
    virtual void init() = 0;

    /**
     * @brief 获取该设备的所有轮询任务
     */
    virtual std::vector<DeviceTask> getTasks() = 0;

    /**
     * @brief 获取设备名称（用于日志输出）
     */
    virtual std::string getName() const = 0;

protected:
    /**
     * @brief 通用的响应失败处理
     */
    void handleFailure(DeviceStatusCpp &status, const std::string &devName) {
        if (status.onFailure(3)) {
            printf("  => [⚠️ 设备离线]: %s 连续3次未读到数据\n", devName.c_str());
        }
    }

    /**
     * @brief 通用的解析失败日志输出 (含原始数据 hex dump)
     */
    void logParseError(int parse_rc, const std::string &devName, const std::string &dataName,
                       const uint8_t *resp, size_t resp_len, uint8_t expectAddr) {
        const char *reason = "未知错误";
        switch (parse_rc) {
            case ParseService::ERR_CRC:    reason = "CRC校验错误"; break;
            case ParseService::ERR_ADDR:   reason = "设备地址不匹配"; break;
            case ParseService::ERR_FUNC:   reason = "功能码不匹配"; break;
            case ParseService::ERR_LEN:    reason = "数据长度不足"; break;
            case ParseService::ERR_FORMAT: reason = "格式错误"; break;
        }
        printf("  => [❌ 解析失败]: %s %s - %s (期望地址0x%02X)\n",
               devName.c_str(), dataName.c_str(), reason, expectAddr);
        // 打印收到的原始数据
        if (resp_len > 0) {
            printf("     原始数据[%zu字节]: ", resp_len);
            size_t dump_len = resp_len > 32 ? 32 : resp_len;
            for (size_t i = 0; i < dump_len; i++) {
                printf("%02X ", resp[i]);
            }
            if (resp_len > 32) printf("...");
            printf("\n");
            if (resp_len >= 1) {
                printf("     响应地址=0x%02X", resp[0]);
                if (resp_len >= 2) printf(" 功能码=0x%02X", resp[1]);
                if (resp_len >= 3) printf(" 字节数=%d", resp[2]);
                printf("\n");
            }
        }
    }
};

#endif /* DEVICE_BASE_HPP */