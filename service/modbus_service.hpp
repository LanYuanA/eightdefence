/**
 * @file modbus_service.hpp
 * @brief 原子服务层 - Modbus通信服务 (C++封装)
 *
 * 封装底层 modbus_core.c 的C函数，提供面向对象的接口。
 */

#ifndef MODBUS_SERVICE_HPP
#define MODBUS_SERVICE_HPP

#include <string>
#include <cstdint>
#include <cstddef>

// 引入底层C接口
extern "C" {
#include "modbus_core.h"
}

class ModbusService {
public:
    /**
     * @brief 构造函数，创建Modbus上下文
     * @param device 串口设备路径 (如 "/dev/ttyS9")
     */
    explicit ModbusService(const char *device);

    /**
     * @brief 析构函数
     */
    ~ModbusService();

    // 禁止拷贝
    ModbusService(const ModbusService &) = delete;
    ModbusService &operator=(const ModbusService &) = delete;

    /* ============================================================
     * Modbus 数据帧操作
     * ============================================================ */

    /**
     * @brief 构建Modbus RTU请求帧并通过串口发送，等待从机响应
     * @param dev_addr  设备地址 (1~247)
     * @param func_code 功能码
     * @param data      数据域 (不含设备地址和功能码)
     * @param data_len  数据域长度
     * @param resp      响应数据输出缓冲区
     * @param resp_max  响应缓冲区最大长度
     * @param resp_len  实际接收到的响应长度
     * @return 0=成功, 其他=失败
     */
    int buildAndSend(uint8_t dev_addr, uint8_t func_code,
                     const uint8_t *data, size_t data_len,
                     uint8_t *resp, size_t resp_max, size_t *resp_len);

    /* ============================================================
     * 寄存器操作 (0x03读/0x06写)
     * ============================================================ */

    /**
     * @brief 读取保持寄存器 (功能码 0x03)
     */
    int readReg(uint8_t dev_addr, uint16_t reg_addr, uint16_t reg_count,
                uint8_t *resp, size_t resp_max, size_t *resp_len);

    /**
     * @brief 写入单个保持寄存器 (功能码 0x06)
     */
    int writeReg(uint8_t dev_addr, uint16_t reg_addr, uint16_t value,
                 uint8_t *resp, size_t resp_max, size_t *resp_len);

    /* ============================================================
     * 线圈操作 (0x01读/0x05写)
     * ============================================================ */

    /**
     * @brief 读取线圈状态 (功能码 0x01)
     */
    int readCoil(uint8_t dev_addr, uint16_t coil_addr, uint16_t coil_count,
                 uint8_t *resp, size_t resp_max, size_t *resp_len);

    /**
     * @brief 写入单个线圈 (功能码 0x05)
     */
    int writeCoil(uint8_t dev_addr, uint16_t coil_addr, bool value,
                  uint8_t *resp, size_t resp_max, size_t *resp_len);

    /**
     * @brief 获取底层Modbus上下文（供兼容用途）
     */

private:
    std::string device_; int baud_;
};

/* ============================================================
 * 底层工具函数 (保留C接口兼容)
 * ============================================================ */
void print_hex_bytes(const uint8_t *data, size_t len);
uint16_t crc16_modbus(const uint8_t *data, size_t len);

#endif /* MODBUS_SERVICE_HPP */