#ifndef PARSE_SERVICE_H
#define PARSE_SERVICE_H

/**
 * @file parse_service.h
 * @brief 原子服务层 - Modbus响应数据解析服务
 *
 * 提供通用的Modbus RTU响应解析能力，包括：
 * - parseDeviceData   : 解析通用设备响应并提取业务值
 * - parseDeviceStatus : 解析并更新通用在线状态与故障状态
 * - parseCrc16        : 校验报文 CRC16 是否正确
 * - parseHexLog       : 将字节流格式化为十六进制调试日志
 *
 * 以及各传感器专用解析子服务：
 * - parsePm25Data / parsePm10Data / parseTemperatureData / parseHumidityData
 * - parseCo2Data / parseTvocData / parseCh2oData / parseO3Data
 * - parseSmokeStatusData / parseWaterStatusData / parseInfraredData
 * - parseRadarData / parseLightData
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 响应解析返回码
 * ============================================================ */
#define PARSE_OK           0
#define PARSE_ERR_FORMAT  -1  /* 协议格式错误 */
#define PARSE_ERR_CRC     -2  /* CRC校验错误 */
#define PARSE_ERR_ADDR    -3  /* 设备地址不匹配 */
#define PARSE_ERR_FUNC    -4  /* 功能码不匹配 */
#define PARSE_ERR_LEN     -5  /* 数据长度不足 */

/* ============================================================
 * 通用解析服务
 * ============================================================ */

/**
 * @brief 解析通用设备响应并提取业务值 (原 parse_validate_response)
 *        验证Modbus RTU响应的基本格式：地址、功能码、CRC、数据长度
 */
int parseDeviceData(const uint8_t *resp, size_t resp_len,
                    uint8_t dev_addr, uint8_t func_code,
                    uint8_t data_bytes);

/**
 * @brief 解析并更新通用在线状态与故障状态
 *        从设备响应中提取设备运行状态信息
 * @param resp      响应数据
 * @param resp_len  响应长度
 * @param dev_addr  设备地址
 * @param online    输出：在线状态 (0=离线, 1=在线)
 * @param fault     输出：故障码 (0=正常)
 * @return PARSE_OK=成功, <0=错误码
 */
int parseDeviceStatus(const uint8_t *resp, size_t resp_len,
                      uint8_t dev_addr, int *online, uint16_t *fault);

/**
 * @brief 从寄存器响应中提取16位无符号数据
 */
uint16_t parseExtractU16(const uint8_t *resp, int word_idx);

/**
 * @brief 从寄存器响应中提取16位有符号数据
 */
int16_t parseExtractS16(const uint8_t *resp, int word_idx);

/**
 * @brief 从线圈响应中提取布尔状态
 */
int parseExtractBool(const uint8_t *resp, int coil_idx);

/* ============================================================
 * 各传感器专用数据解析子服务
 * ============================================================ */

/** @brief 解析 PM2.5 数据 */
int parsePm25Data(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析 PM10 数据 */
int parsePm10Data(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析湿度数据 */
int parseHumidityData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析温度数据 */
int parseTemperatureData(const uint8_t *resp, size_t resp_len, int16_t *value);

/** @brief 解析 TVOC 数据 */
int parseTvocData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析甲醛数据 */
int parseCh2oData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析臭氧数据 */
int parseO3Data(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析二氧化碳数据 */
int parseCo2Data(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析压力值 */
int parsePressData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析压力故障状态 */
int parsePressFaultData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析烟雾状态数据 */
int parseSmokeStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析水浸状态数据 */
int parseWaterStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析红外状态数据 */
int parseInfraredData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析雷达状态数据 */
int parseRadarData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/** @brief 解析光照数据 */
int parseLightData(const uint8_t *resp, size_t resp_len, uint16_t *value);

/* ============================================================
 * 兼容性包装函数声明 (旧API名称)
 * ============================================================ */

/** @brief parseDeviceData 的旧名称别名 */
int parseValidateResp(const uint8_t *resp, size_t resp_len,
                      uint8_t dev_addr, uint8_t func_code,
                      uint8_t data_bytes);

/** @brief parseExtractU16 的旧名称别名 */
uint16_t parseU16(const uint8_t *resp, int word_idx);

/** @brief parseExtractS16 的旧名称别名 */
int16_t parseS16(const uint8_t *resp, int word_idx);

#ifdef __cplusplus
}
#endif

#endif /* PARSE_SERVICE_H */
