#ifndef MODBUS_CORE_H
#define MODBUS_CORE_H

#include <stddef.h>
#include <stdint.h>

int modbus_send_and_recv_hex(const char *device, int baud, const char *request_hex,
                             uint8_t *response_buf, size_t response_cap,
                             size_t *response_len, int timeout_ms);

int modbus_build_and_send(const char *device, int baud,
                          uint8_t addr, uint8_t func,
                          uint16_t reg, uint16_t data,
                          uint8_t *response_buf, size_t response_cap,
                          size_t *response_len, int timeout_ms);

uint16_t crc16_modbus(const uint8_t *data, size_t len);

void print_hex_bytes(const uint8_t *data, size_t len);

#endif // MODBUS_CORE_H
