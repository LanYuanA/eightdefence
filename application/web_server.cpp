#include "web_server.h"
#include "core/global_devices.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <string>
#include <vector>

#define PORT 8080

// 辅助函数判断是否以某字符串结尾
bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

void send_response(int client_socket, const char *header, const char *content_type, const char *body, size_t body_len) {
    char headers[1024];
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "\r\n", header, content_type, body_len);
    write(client_socket, headers, strlen(headers));
    write(client_socket, body, body_len);
}

// 提取查询参数
std::string get_query_param(const std::string& url, const std::string& key) {
    size_t pos = url.find(key + "=");
    if (pos == std::string::npos) return "";
    pos += key.length() + 1;
    size_t end = url.find('&', pos);
    if (end == std::string::npos) end = url.find(' ', pos);
    return url.substr(pos, end - pos);
}

void serve_file(int client_socket, const std::string& filepath) {
    FILE *f = fopen(filepath.c_str(), "r");
    if (!f) {
        const char *err = "404 Not Found";
        send_response(client_socket, "404 Not Found", "text/plain", err, strlen(err));
        return;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = (char*)malloc(fsize + 1);
    fread(data, 1, fsize, f);
    fclose(f);
    data[fsize] = 0;

    const char* content_type = "text/plain";
    if (ends_with(filepath, ".html")) content_type = "text/html; charset=utf-8";
    else if (ends_with(filepath, ".js")) content_type = "application/javascript; charset=utf-8";
    else if (ends_with(filepath, ".css")) content_type = "text/css; charset=utf-8";
    else if (ends_with(filepath, ".json")) content_type = "application/json; charset=utf-8";

    send_response(client_socket, "200 OK", content_type, data, fsize);
    free(data);
}

void* start_web_server(void *arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[4096] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return NULL;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        return NULL;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return NULL;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return NULL;
    }
    printf("🌐 [HTTP 服务器] 运行在 http://localhost:%d\n", PORT);

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int read_len = read(new_socket, buffer, sizeof(buffer) - 1);
        if (read_len <= 0) {
            close(new_socket);
            continue;
        }
        
        std::string req(buffer);
        
        if (req.find("GET /api/data") == 0) {
            char json[4096];
            snprintf(json, sizeof(json),
                "{"
                "\"pm25\": %u, \"pm25_online\": %d, "
                "\"pm10\": %u, \"pm10_online\": %d, "
                "\"humidity\": %u, \"humidity_online\": %d, "
                "\"temperature\": %d, \"temperature_online\": %d, "
                "\"tvoc\": %u, \"tvoc_online\": %d, "
                "\"ch2o\": %u, \"ch2o_online\": %d, "
                "\"o3\": %u, \"o3_online\": %d, "
                "\"co2\": %u, \"co2_online\": %d, "
                "\"smoke\": %d, \"smoke_online\": %d, "
                "\"water\": %d, \"water_online\": %d, "
                "\"ir\": %d, \"radar\": %d, \"ir_online\": %d, "
                "\"lux\": %u, \"light_online\": %d, "
                "\"ac_online\": %d, "
                "\"humidifier_power\": %d, \"humidifier_fault\": %d, \"humidifier_online\": %d, "
                "\"purifier_power\": %d, \"purifier_run_mode\": %d, \"purifier_online\": %d"
                "}",
                dev_pm25.getValue(), dev_pm25.isOnline(),
                dev_pm10.getValue(), dev_pm10.isOnline(),
                dev_humidity.getValue(), dev_humidity.isOnline(),
                dev_temperature.getValue(), dev_temperature.isOnline(),
                dev_tvoc.getValue(), dev_tvoc.isOnline(),
                dev_ch2o.getValue(), dev_ch2o.isOnline(),
                dev_o3.getValue(), dev_o3.isOnline(),
                dev_co2.getValue(), dev_co2.isOnline(),
                dev_smoke.getAlarmState(), dev_smoke.isOnline(),
                dev_water.getWaterState(), dev_water.isOnline(),
                dev_infrared.getInfraredState(), dev_infrared.getRadarState(), dev_infrared.isOnline(),
                dev_light.getIlluminance(), dev_light.isOnline(),
                dev_ac.isOnline(),
                dev_humidifier.getPowerState(), dev_humidifier.getFaultState(), dev_humidifier.isOnline(),
                dev_purifier.getPowerStatus(), dev_purifier.getRunMode(), dev_purifier.isOnline()
            );
            send_response(new_socket, "200 OK", "application/json; charset=utf-8", json, strlen(json));
            
        } else if (req.find("GET /api/control") == 0) {
            std::string device = get_query_param(req, "device");
            std::string action = get_query_param(req, "action");
            std::string value_str = get_query_param(req, "val");
            uint16_t val = value_str.empty() ? 0 : std::stoi(value_str);
            
            uint8_t resp[512];
            size_t resp_len = 0;
            int rc = -1;
            
            if (device == "ac" && g_modbus != nullptr) {
                if (action == "cool_on") rc = dev_ac.setCoolOn(*g_modbus, resp, &resp_len);
                else if (action == "cool_off") rc = dev_ac.setCoolOff(*g_modbus, resp, &resp_len);
                else if (action == "heat_on") rc = dev_ac.setHeatOn(*g_modbus, resp, &resp_len);
                else if (action == "heat_off") rc = dev_ac.setHeatOff(*g_modbus, resp, &resp_len);
            } else if (device == "humidifier" && g_modbus != nullptr) {
                if (action == "power") rc = dev_humidifier.setPower(*g_modbus, val, resp, &resp_len);
                else if (action == "dehumidify") rc = dev_humidifier.setDehumidify(*g_modbus, val, resp, &resp_len);
                else if (action == "humidify") rc = dev_humidifier.setHumidify(*g_modbus, val, resp, &resp_len);
                else if (action == "purify") rc = dev_humidifier.setPurify(*g_modbus, val, resp, &resp_len);
                else if (action == "const_hum") rc = dev_humidifier.setConstHum(*g_modbus, val, resp, &resp_len);
            } else if (device == "purifier" && g_modbus != nullptr) {
                if (action == "power") rc = dev_purifier.setPower(*g_modbus, val, resp, &resp_len);
                else if (action == "run_mode") rc = dev_purifier.setRunMode(*g_modbus, val, resp, &resp_len);
                else if (action == "manual") rc = dev_purifier.setManual(*g_modbus, val, resp, &resp_len);
            }
            
            const char* msg = (rc == 0) ? "{\"status\":\"success\"}" : "{\"status\":\"failed\"}";
            send_response(new_socket, "200 OK", "application/json; charset=utf-8", msg, strlen(msg));
            
        } else if (req.find("GET /assets/") == 0) {
            // Serve static files inside /assets/
            size_t start = req.find("/assets/");
            size_t end = req.find(" HTTP/");
            if (start != std::string::npos && end != std::string::npos) {
                std::string path = "ui/dist" + req.substr(start, end - start);
                serve_file(new_socket, path);
            }
        } else {
            // 所有其他路由返回 dashboard.html 以支持 Vue Router 的 History 模式
            serve_file(new_socket, "dashboard.html");
        }
        close(new_socket);
    }
    return NULL;
}
