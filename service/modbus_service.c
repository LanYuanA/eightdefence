#include "modbus_service.h"
#include "modbus_core.h"

#include <stdio.h>
#include <string.h>

/* ============================================================
 * 原子服务层 - Modbus通信服务实现
 * ============================================================ */

ModbusContext createContext(const char *device) {
    ModbusContext ctx;
    ctx.device   = device;
    ctx.baudrate = 9600;
    return ctx;
}

uint16_t getCrc16(const uint8_t *data, size_t len) {
    return crc16_modbus(data, len);
}

int parseCrc16(const uint8_t *data, size_t len) {
    if (len < 4) return -1;
    uint16_t received = data[len - 2] | (data[len - 1] << 8);
    uint16_t calc = crc16_modbus(data, len - 2);
    return (received == calc) ? 0 : -1;
}

void parseHexLog(const uint8_t *data, size_t len, char *buf, size_t buf_size) {
    size_t pos = 0;
    for (size_t i = 0; i < len && pos + 4 < buf_size; i++) {
        pos += snprintf(buf + pos, buf_size - pos, "%02X ", data[i]);
    }
    if (pos > 0 && pos < buf_size) buf[pos - 1] = '\0';
}

int sendModbusFrame(ModbusContext *ctx, const uint8_t *data, size_t len) {
    (void)ctx;
    (void)data;
    (void)len;
    /* 此函数暂不单独使用，由sendAndRecvModbus统一处理 */
    return -1;
}

int recvModbusFrame(ModbusContext *ctx, uint8_t *resp, size_t resp_size,
                    size_t *resp_len) {
    (void)ctx;
    (void)resp;
    (void)resp_size;
    (void)resp_len;
    /* 此函数暂不单独使用，由sendAndRecvModbus统一处理 */
    return -1;
}

int sendAndRecvModbus(ModbusContext *ctx, const uint8_t *req, size_t req_len,
                      uint8_t *resp, size_t resp_size, size_t *resp_len) {
    int rc = sendModbusFrame(ctx, req, req_len);
    if (rc != 0) return rc;
    return recvModbusFrame(ctx, resp, resp_size, resp_len);
}

int sendDeviceCommand(ModbusContext *ctx, uint8_t addr, uint16_t reg,
                      uint16_t count, uint8_t *resp, size_t resp_size,
                      size_t *resp_len) {
    return modbus_build_and_send(ctx->device, ctx->baudrate, addr,
                                 0x03, reg, count, resp, resp_size,
                                 resp_len, 300);
}

int readHoldingRegisters(ModbusContext *ctx, uint8_t addr, uint16_t reg,
                         uint16_t count, uint8_t *resp, size_t resp_size,
                         size_t *resp_len) {
    return sendDeviceCommand(ctx, addr, reg, count, resp, resp_size, resp_len);
}

int writeSingleRegister(ModbusContext *ctx, uint8_t addr, uint16_t reg,
                        uint16_t value, uint8_t *resp, size_t resp_size,
                        size_t *resp_len) {
    return modbus_build_and_send(ctx->device, ctx->baudrate, addr,
                                 0x06, reg, value, resp, resp_size,
                                 resp_len, 300);
}

int readCoils(ModbusContext *ctx, uint8_t addr, uint16_t coil,
              uint16_t count, uint8_t *resp, size_t resp_size,
              size_t *resp_len) {
    return modbus_build_and_send(ctx->device, ctx->baudrate, addr,
                                 0x01, coil, count, resp, resp_size,
                                 resp_len, 300);
}

int writeSingleCoil(ModbusContext *ctx, uint8_t addr, uint16_t coil,
                    uint16_t value, uint8_t *resp, size_t resp_size,
                    size_t *resp_len) {
    return modbus_build_and_send(ctx->device, ctx->baudrate, addr,
                                 0x05, coil, value, resp, resp_size,
                                 resp_len, 300);
}

/* ============================================================
 * 设备层便捷函数
 * ============================================================ */
int modbusReadReg(ModbusContext *ctx, uint8_t dev_addr, uint16_t reg_addr,
                  uint16_t reg_count, uint8_t *resp, size_t resp_sz,
                  size_t *resp_len) {
    return readHoldingRegisters(ctx, dev_addr, reg_addr, reg_count,
                                resp, resp_sz, resp_len);
}

int modbusWriteReg(ModbusContext *ctx, uint8_t dev_addr, uint16_t reg_addr,
                   uint16_t value, uint8_t *resp, size_t resp_sz,
                   size_t *resp_len) {
    return writeSingleRegister(ctx, dev_addr, reg_addr, value,
                               resp, resp_sz, resp_len);
}

int modbusReadCoil(ModbusContext *ctx, uint8_t dev_addr, uint16_t coil_addr,
                   uint16_t coil_count, uint8_t *resp, size_t resp_sz,
                   size_t *resp_len) {
    return readCoils(ctx, dev_addr, coil_addr, coil_count,
                     resp, resp_sz, resp_len);
}

int modbusWriteCoil(ModbusContext *ctx, uint8_t dev_addr, uint16_t coil_addr,
                    uint16_t value, uint8_t *resp, size_t resp_sz,
                    size_t *resp_len) {
    return writeSingleCoil(ctx, dev_addr, coil_addr, value,
                           resp, resp_sz, resp_len);
}
