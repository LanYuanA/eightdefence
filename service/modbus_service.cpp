#include "modbus_service.hpp"
#include <cstdio>
#include <cstring>
#include <string>

ModbusService::ModbusService(const char *device) : device_(device), baud_(9600) {}

ModbusService::~ModbusService() {}

int ModbusService::buildAndSend(uint8_t dev_addr, uint8_t func_code,
                                 const uint8_t *data, size_t data_len,
                                 uint8_t *resp, size_t resp_max, size_t *resp_len) {
    char hex_cmd[1024] = {0};
    char temp[4];
    snprintf(hex_cmd, sizeof(hex_cmd), "%02X%02X", dev_addr, func_code);
    for (size_t i = 0; i < data_len; i++) {
        snprintf(temp, sizeof(temp), "%02X", data[i]);
        strcat(hex_cmd, temp);
    }
    return modbus_send_and_recv_hex(device_.c_str(), baud_, hex_cmd, resp, resp_max, resp_len, 1000);
}

int ModbusService::readReg(uint8_t dev_addr, uint16_t reg_addr, uint16_t reg_count,
                            uint8_t *resp, size_t resp_max, size_t *resp_len) {
    return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x03, reg_addr, reg_count, resp, resp_max, resp_len, 1000);
}

int ModbusService::writeReg(uint8_t dev_addr, uint16_t reg_addr, uint16_t value,
                             uint8_t *resp, size_t resp_max, size_t *resp_len) {
    return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x06, reg_addr, value, resp, resp_max, resp_len, 1000);
}

int ModbusService::readCoil(uint8_t dev_addr, uint16_t coil_addr, uint16_t coil_count,
                              uint8_t *resp, size_t resp_max, size_t *resp_len) {
    return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x01, coil_addr, coil_count, resp, resp_max, resp_len, 1000);
}

int ModbusService::writeCoil(uint8_t dev_addr, uint16_t coil_addr, bool value,
                               uint8_t *resp, size_t resp_max, size_t *resp_len) {
    uint16_t coil_val = value ? 0xFF00 : 0x0000;
    return modbus_build_and_send(device_.c_str(), baud_, dev_addr, 0x05, coil_addr, coil_val, resp, resp_max, resp_len, 1000);
}
