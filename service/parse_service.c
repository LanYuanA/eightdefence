#include "parse_service.h"
#include "modbus_core.h"

/* ============================================================
 * 原子服务层 - Modbus响应数据解析服务实现
 * ============================================================ */

/* --- 通用解析服务 --- */

int parseDeviceData(const uint8_t *resp, size_t resp_len,
                    uint8_t dev_addr, uint8_t func_code,
                    uint8_t data_bytes) {
    if (resp_len < 5) {
        return PARSE_ERR_LEN;
    }
    if (resp[0] != dev_addr) {
        return PARSE_ERR_ADDR;
    }
    if (resp[1] != func_code) {
        return PARSE_ERR_FUNC;
    }
    if (data_bytes > 0 && resp[2] != data_bytes) {
        return PARSE_ERR_FORMAT;
    }

    /* CRC16 校验 */
    uint16_t expected_crc = crc16_modbus(resp, resp_len - 2);
    uint16_t actual_crc   = resp[resp_len - 2] | (resp[resp_len - 1] << 8);
    if (expected_crc != actual_crc) {
        return PARSE_ERR_CRC;
    }

    return PARSE_OK;
}

int parseDeviceStatus(const uint8_t *resp, size_t resp_len,
                      uint8_t dev_addr, int *online, uint16_t *fault) {
    if (!online || !fault) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, dev_addr, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    /* 在线状态：收到响应即表示在线 */
    *online = 1;
    /* 故障码：从最后一个字读取 */
    if (resp_len >= 7) {
        *fault = parseExtractU16(resp, (resp[2] / 2) - 1);
    } else {
        *fault = 0;
    }
    return PARSE_OK;
}

uint16_t parseExtractU16(const uint8_t *resp, int word_idx) {
    /* resp[0]=addr, resp[1]=func, resp[2]=byte_count, resp[3..]=data */
    int offset = 3 + word_idx * 2;
    return (uint16_t)((resp[offset] << 8) | resp[offset + 1]);
}

int16_t parseExtractS16(const uint8_t *resp, int word_idx) {
    return (int16_t)parseExtractU16(resp, word_idx);
}

int parseExtractBool(const uint8_t *resp, int coil_idx) {
    /* resp[0]=addr, resp[1]=func, resp[2]=byte_count, resp[3..]=data */
    int byte_idx = 3 + (coil_idx / 8);
    int bit_idx  = coil_idx % 8;
    return (resp[byte_idx] >> bit_idx) & 0x01;
}

/* --- 各传感器专用数据解析子服务 --- */

int parsePm25Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parsePm10Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parseHumidityData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parseTemperatureData(const uint8_t *resp, size_t resp_len, int16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractS16(resp, 0);
    return PARSE_OK;
}

int parseTvocData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parseCh2oData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parseO3Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parseCo2Data(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parsePressData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parsePressFaultData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

int parseSmokeStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != PARSE_OK) return rc;
    *value = (uint16_t)parseExtractBool(resp, 0);
    return PARSE_OK;
}

int parseWaterStatusData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != PARSE_OK) return rc;
    *value = (uint16_t)parseExtractBool(resp, 0);
    return PARSE_OK;
}

int parseInfraredData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != PARSE_OK) return rc;
    *value = (uint16_t)parseExtractBool(resp, 0);
    return PARSE_OK;
}

int parseRadarData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x01, 0);
    if (rc != PARSE_OK) return rc;
    *value = (uint16_t)parseExtractBool(resp, 0);
    return PARSE_OK;
}

int parseLightData(const uint8_t *resp, size_t resp_len, uint16_t *value) {
    if (!value) return PARSE_ERR_FORMAT;
    int rc = parseDeviceData(resp, resp_len, 0, 0x03, 0);
    if (rc != PARSE_OK) return rc;
    *value = parseExtractU16(resp, 0);
    return PARSE_OK;
}

/* ============================================================
 * 兼容性包装函数 (旧API名称 -> 新API名称)
 * ============================================================ */

int parseValidateResp(const uint8_t *resp, size_t resp_len,
                      uint8_t dev_addr, uint8_t func_code,
                      uint8_t data_bytes) {
    return parseDeviceData(resp, resp_len, dev_addr, func_code, data_bytes);
}

uint16_t parseU16(const uint8_t *resp, int word_idx) {
    return parseExtractU16(resp, word_idx);
}

int16_t parseS16(const uint8_t *resp, int word_idx) {
    return parseExtractS16(resp, word_idx);
}
