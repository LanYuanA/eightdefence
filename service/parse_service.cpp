/**
 * @file parse_service.cpp
 * @brief 原子服务层 - Modbus响应数据解析服务实现 (C++封装)
 */

#include "parse_service.hpp"

extern "C" {
#include "modbus_core.h"
}

/* ============================================================
 * 通用解析服务
 * ============================================================ */

int ParseService::parseDeviceData(const uint8_t *resp, size_t resp_len,
                                   uint8_t dev_addr, uint8_t func_code,
                                   uint8_t data_bytes) {
    if (resp_len < 5) {
        return ERR_LEN;
    }
    if (resp[0] != dev_addr) {
        return ERR_ADDR;
    }
    if (resp[1] != func_code) {
        return ERR_FUNC;
    }
    if (data_bytes > 0 && resp[2] != data_bytes) {
        return ERR_FORMAT;
    }

    /* CRC16 校验 */
    uint16_t expected_crc = crc16_modbus(resp, resp_len - 2);
    uint16_t actual_crc   = resp[resp_len - 2] | (resp[resp_len - 1] << 8);
    if (expected_crc != actual_crc) {
        return ERR_CRC;
    }

    return OK;
}

int ParseService::parseDeviceStatus(const uint8_t *resp, size_t resp_len,
                                     uint8_t dev_addr, int *online, uint16_t *fault) {
    if (!online || !fault) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, dev_addr, 0x03, 0);
    if (rc != OK) return rc;
    /* 在线状态：收到响应即表示在线 */
    *online = 1;
    /* 故障码：从最后一个字读取 */
    if (resp_len >= 7) {
        *fault = extractU16(resp, (resp[2] / 2) - 1);
    } else {
        *fault = 0;
    }
    return OK;
}

uint16_t ParseService::extractU16(const uint8_t *resp, int word_idx) {
    /* resp[0]=addr, resp[1]=func, resp[2]=byte_count, resp[3..]=data */
    int offset = 3 + word_idx * 2;
    return static_cast<uint16_t>((resp[offset] << 8) | resp[offset + 1]);
}

int16_t ParseService::extractS16(const uint8_t *resp, int word_idx) {
    return static_cast<int16_t>(extractU16(resp, word_idx));
}

int ParseService::extractBool(const uint8_t *resp, int coil_idx) {
    /* resp[0]=addr, resp[1]=func, resp[2]=byte_count, resp[3..]=data */
    int byte_idx = 3 + (coil_idx / 8);
    int bit_idx  = coil_idx % 8;
    return (resp[byte_idx] >> bit_idx) & 0x01;
}

/* ============================================================
 * 各传感器专用数据解析子服务
 * ============================================================ */

int ParseService::parsePm25Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parsePm10Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parseHumidityData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parseTemperatureData(const uint8_t *resp, size_t resp_len, int16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractS16(resp, 0);
    return OK;
}

int ParseService::parseTvocData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parseCh2oData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parseO3Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parseCo2Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parsePressData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parsePressFaultData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}

int ParseService::parseSmokeStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != OK) return rc;
    *value = static_cast<uint16_t>(extractBool(resp, 0));
    return OK;
}

int ParseService::parseWaterStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != OK) return rc;
    *value = static_cast<uint16_t>(extractBool(resp, 0));
    return OK;
}

int ParseService::parseInfraredData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != OK) return rc;
    *value = static_cast<uint16_t>(extractBool(resp, 0));
    return OK;
}

int ParseService::parseRadarData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != OK) return rc;
    *value = static_cast<uint16_t>(extractBool(resp, 0));
    return OK;
}

int ParseService::parseLightData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != OK) return rc;
    *value = extractU16(resp, 0);
    return OK;
}