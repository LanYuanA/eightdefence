#ifndef MODBUS_SERVICE_H
#define MODBUS_SERVICE_H

/**
 * @file modbus_service.h
 * @brief 原子服务层 - Modbus通信服务
 *
 * 提供与硬件无关的Modbus RTU通信能力，包括：
 * - sendModbusFrame   : 发送一帧 Modbus 请求报文
 * - recvModbusFrame   : 接收一帧 Modbus 响应报文
 * - sendAndRecvModbus : 执行一次完整的发送与接收流程
 * - sendDeviceCommand : 根据设备地址与寄存器信息下发指令
 * - getCrc16          : 计算报文 CRC16 校验值
 * - parseCrc16        : 校验报文 CRC16 是否正确
 * - parseHexLog       : 将字节流格式化为十六进制调试日志
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_BUFFER_SIZE 512

/**
 * @brief Modbus上下文
 */
typedef struct {
    const char *device;     /* 串口设备路径，如 "/dev/ttyS9" */
    int         baudrate;   /* 波特率，通常 9600 */
} ModbusContext;

/**
 * @brief 创建默认的Modbus上下文
 */
ModbusContext createContext(const char *device);

/**
 * @brief 计算报文 CRC16 校验值
 * @param data 数据缓冲区
 * @param len  数据长度
 * @return CRC16 值
 */
uint16_t getCrc16(const uint8_t *data, size_t len);

/**
 * @brief 校验报文 CRC16 是否正确
 * @param data 数据缓冲区（含CRC）
 * @param len  数据总长度（含CRC的2字节）
 * @return 0=校验通过, <0=校验失败
 */
int parseCrc16(const uint8_t *data, size_t len);

/**
 * @brief 将字节流格式化为十六进制调试日志
 * @param data  数据缓冲区
 * @param len   数据长度
 * @param buf   输出字符串缓冲区
 * @param buf_size 输出缓冲区大小
 */
void parseHexLog(const uint8_t *data, size_t len, char *buf, size_t buf_size);

/**
 * @brief 发送一帧 Modbus 请求报文
 * @param ctx  Modbus上下文
 * @param data 报文数据
 * @param len  报文长度
 * @return 0=成功, <0=失败
 */
int sendModbusFrame(ModbusContext *ctx, const uint8_t *data, size_t len);

/**
 * @brief 接收一帧 Modbus 响应报文
 * @param ctx       Modbus上下文
 * @param resp      响应缓冲区
 * @param resp_size 缓冲区大小
 * @param resp_len  实际响应长度 (输出)
 * @return 0=成功, <0=失败
 */
int recvModbusFrame(ModbusContext *ctx, uint8_t *resp, size_t resp_size,
                    size_t *resp_len);

/**
 * @brief 执行一次完整的发送与接收流程
 * @param ctx       Modbus上下文
 * @param req       请求报文
 * @param req_len   请求长度
 * @param resp      响应缓冲区
 * @param resp_size 缓冲区大小
 * @param resp_len  实际响应长度 (输出)
 * @return 0=成功, <0=失败
 */
int sendAndRecvModbus(ModbusContext *ctx, const uint8_t *req, size_t req_len,
                      uint8_t *resp, size_t resp_size, size_t *resp_len);

/**
 * @brief 根据设备地址与寄存器信息下发读取指令 (功能码 0x03)
 * @param ctx       Modbus上下文
 * @param addr      设备地址 (从站地址)
 * @param reg       起始寄存器地址
 * @param count     读取寄存器数量
 * @param resp      响应缓冲区
 * @param resp_size 缓冲区大小
 * @param resp_len  实际响应长度 (输出)
 * @return 0=成功, <0=失败
 */
int sendDeviceCommand(ModbusContext *ctx, uint8_t addr, uint16_t reg,
                      uint16_t count, uint8_t *resp, size_t resp_size,
                      size_t *resp_len);

/**
 * @brief 读取保持寄存器 (功能码 0x03) - sendDeviceCommand的别名
 */
int readHoldingRegisters(ModbusContext *ctx, uint8_t addr, uint16_t reg,
                         uint16_t count, uint8_t *resp, size_t resp_size,
                         size_t *resp_len);

/**
 * @brief 写入单个寄存器 (功能码 0x06)
 */
int writeSingleRegister(ModbusContext *ctx, uint8_t addr, uint16_t reg,
                        uint16_t value, uint8_t *resp, size_t resp_size,
                        size_t *resp_len);

/**
 * @brief 读取线圈状态 (功能码 0x01)
 */
int readCoils(ModbusContext *ctx, uint8_t addr, uint16_t coil,
              uint16_t count, uint8_t *resp, size_t resp_size,
              size_t *resp_len);

/**
 * @brief 写入单个线圈 (功能码 0x05)
 */
int writeSingleCoil(ModbusContext *ctx, uint8_t addr, uint16_t coil,
                    uint16_t value, uint8_t *resp, size_t resp_size,
                    size_t *resp_len);

/* --- 设备层便捷函数 --- */

/**
 * @brief 读取保持寄存器 (设备层便捷函数)
 */
int modbusReadReg(ModbusContext *ctx, uint8_t dev_addr, uint16_t reg_addr,
                  uint16_t reg_count, uint8_t *resp, size_t resp_sz,
                  size_t *resp_len);

/**
 * @brief 写入单个寄存器 (设备层便捷函数)
 */
int modbusWriteReg(ModbusContext *ctx, uint8_t dev_addr, uint16_t reg_addr,
                   uint16_t value, uint8_t *resp, size_t resp_sz,
                   size_t *resp_len);

/**
 * @brief 读取线圈状态 (设备层便捷函数)
 */
int modbusReadCoil(ModbusContext *ctx, uint8_t dev_addr, uint16_t coil_addr,
                   uint16_t coil_count, uint8_t *resp, size_t resp_sz,
                   size_t *resp_len);

/**
 * @brief 写入单个线圈 (设备层便捷函数)
 */
int modbusWriteCoil(ModbusContext *ctx, uint8_t dev_addr, uint16_t coil_addr,
                    uint16_t value, uint8_t *resp, size_t resp_sz,
                    size_t *resp_len);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_SERVICE_H */