/**
 * @file parse_service.hpp
 * @brief 原子服务层 - Modbus响应数据解析服务 (C++封装)
 */

#ifndef PARSE_SERVICE_HPP
#define PARSE_SERVICE_HPP

#include <cstdint>
#include <cstddef>

class ParseService {
public:
    /* ============================================================
     * 响应解析返回码
     * ============================================================ */
    static constexpr int OK          =  0;
    static constexpr int ERR_FORMAT  = -1;
    static constexpr int ERR_CRC     = -2;
    static constexpr int ERR_ADDR    = -3;
    static constexpr int ERR_FUNC    = -4;
    static constexpr int ERR_LEN     = -5;

    /* ============================================================
     * 通用解析服务
     * ============================================================ */

    /**
     * @brief 解析通用设备响应并验证格式
     *        验证Modbus RTU响应的基本格式：地址、功能码、CRC、数据长度
     */
    static int parseDeviceData(const uint8_t *resp, size_t resp_len,
                               uint8_t dev_addr, uint8_t func_code,
                               uint8_t data_bytes);

    /**
     * @brief 解析并更新通用在线状态与故障状态
     */
    static int parseDeviceStatus(const uint8_t *resp, size_t resp_len,
                                 uint8_t dev_addr, int *online, uint16_t *fault);

    /**
     * @brief 从寄存器响应中提取16位无符号数据
     */
    static uint16_t extractU16(const uint8_t *resp, int word_idx);

    /**
     * @brief 从寄存器响应中提取16位有符号数据
     */
    static int16_t extractS16(const uint8_t *resp, int word_idx);

    /**
     * @brief 从线圈响应中提取布尔状态
     */
    static int extractBool(const uint8_t *resp, int coil_idx);

    /* ============================================================
     * 各传感器专用数据解析子服务
     * ============================================================ */
    static int parsePm25Data(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parsePm10Data(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseHumidityData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseTemperatureData(const uint8_t *resp, size_t resp_len, int16_t *value);
    static int parseTvocData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseCh2oData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseO3Data(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseCo2Data(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parsePressData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parsePressFaultData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseSmokeStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseWaterStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseInfraredData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseRadarData(const uint8_t *resp, size_t resp_len, uint16_t *value);
    static int parseLightData(const uint8_t *resp, size_t resp_len, uint16_t *value);
};

#endif /* PARSE_SERVICE_HPP */