#include "modbus_service.hpp"
#include "serial_bus.hpp"
#include <cstdio>
#include <cstring>
#include <string>

ModbusService::ModbusService(const char *device)
    : device_(device), baud_(9600), bus_(nullptr) {}

ModbusService::ModbusService(SerialBus *bus)
    : device_(bus ? bus->getDevice() : ""), baud_(bus ? bus->getBaud() : 9600), bus_(bus) {}

ModbusService::~ModbusService() {}

int ModbusService::buildAndSend(uint8_t dev_addr, uint8_t func_code,
                                 const uint8_t *data, size_t data_len,
                                 uint8_t *resp, size_t resp_max, size_t *resp_len,
                                 bool isWrite) {
    if (bus_) {
        // SerialBus 模式: 构建帧, 通过 SerialBus 线程安全发送
        // 构建 hex 命令字符串, 然后使用 transactHex
        char hex_cmd[1024] = {0};
        char temp[4];
        snprintf(hex_cmd, sizeof(hex_cmd), "%02X%02X", dev_addr, func_code);
        for (size_t i = 0; i < data_len; i++) {
            snprintf(temp, sizeof(temp), "%02X", data[i]);
            strcat(hex_cmd, temp);
        }
        return bus_->transactHex(hex_cmd, resp, resp_max, resp_len, 1000, isWrite);
    } else {
        // 传统模式: 直接调用底层C函数
        char hex_cmd[1024] = {0};
        char temp[4];
        snprintf(hex_cmd, sizeof(hex_cmd), "%02X%02X", dev_addr, func_code);
        for (size_t i = 0; i < data_len; i++) {
            snprintf(temp, sizeof(temp), "%02X", data[i]);
            strcat(hex_cmd, temp);
        }
        return modbus_send_and_recv_hex(device_.c_str(), baud_, hex_cmd, resp, resp_max, resp_len, 1000);
    }
}

int ModbusService::readReg(uint8_t dev_addr, uint16_t reg_addr, uint16_t reg_count,
                            uint8_t *resp, size_t resp_max, size_t *resp_len) {
    if (bus_) {
        // SerialBus 模式: 构建完整的 Modbus RTU 帧
        uint8_t frame[256];
        frame[0] = dev_addr;
        frame[1] = 0x03;
        frame[2] = (reg_addr >> 8) & 0xFF;
        frame[3] = reg_addr & 0xFF;
        frame[4] = (reg_count >> 8) & 0xFF;
        frame[5] = reg_count & 0xFF;
        uint16_t crc = crc16_modbus(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;
        return bus_->transact(frame, 8, resp, resp_max, resp_len, 1000, false);  // 读操作
    } else {
        return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x03, reg_addr, reg_count, resp, resp_max, resp_len, 1000);
    }
}

int ModbusService::writeReg(uint8_t dev_addr, uint16_t reg_addr, uint16_t value,
                             uint8_t *resp, size_t resp_max, size_t *resp_len) {
    if (bus_) {
        uint8_t frame[256];
        frame[0] = dev_addr;
        frame[1] = 0x06;
        frame[2] = (reg_addr >> 8) & 0xFF;
        frame[3] = reg_addr & 0xFF;
        frame[4] = (value >> 8) & 0xFF;
        frame[5] = value & 0xFF;
        uint16_t crc = crc16_modbus(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;
        return bus_->transact(frame, 8, resp, resp_max, resp_len, 1000, true);  // 写操作, 优先抢占
    } else {
        return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x06, reg_addr, value, resp, resp_max, resp_len, 1000);
    }
}

int ModbusService::readCoil(uint8_t dev_addr, uint16_t coil_addr, uint16_t coil_count,
                              uint8_t *resp, size_t resp_max, size_t *resp_len) {
    if (bus_) {
        uint8_t frame[256];
        frame[0] = dev_addr;
        frame[1] = 0x01;
        frame[2] = (coil_addr >> 8) & 0xFF;
        frame[3] = coil_addr & 0xFF;
        frame[4] = (coil_count >> 8) & 0xFF;
        frame[5] = coil_count & 0xFF;
        uint16_t crc = crc16_modbus(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;
        return bus_->transact(frame, 8, resp, resp_max, resp_len, 1000, false);  // 读操作
    } else {
        return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x01, coil_addr, coil_count, resp, resp_max, resp_len, 1000);
    }
}

int ModbusService::writeCoil(uint8_t dev_addr, uint16_t coil_addr, bool value,
                               uint8_t *resp, size_t resp_max, size_t *resp_len) {
    uint16_t coil_val = value ? 0xFF00 : 0x0000;
    if (bus_) {
        uint8_t frame[256];
        frame[0] = dev_addr;
        frame[1] = 0x05;
        frame[2] = (coil_addr >> 8) & 0xFF;
        frame[3] = coil_addr & 0xFF;
        frame[4] = (coil_val >> 8) & 0xFF;
        frame[5] = coil_val & 0xFF;
        uint16_t crc = crc16_modbus(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;
        return bus_->transact(frame, 8, resp, resp_max, resp_len, 1000, true);  // 写操作, 优先抢占
    } else {
        return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x05, coil_addr, coil_val, resp, resp_max, resp_len, 1000);
    }
}