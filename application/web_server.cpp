#include "web_server.h"
#include "core/global_devices.hpp"
#include "core/device_config.h"
#include "application/app_manager.hpp"

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
#include <dirent.h>
#include <algorithm>
#include <sys/stat.h>

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

void send_http_response(int client_socket, const HttpResponse& resp) {
    char statusStr[64];
    snprintf(statusStr, sizeof(statusStr), "%d %s", resp.statusCode, resp.statusText.c_str());
    send_response(client_socket, statusStr, resp.contentType.c_str(),
                  resp.body.c_str(), resp.body.size());
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

// 从 HTTP 请求行中提取路径和查询字符串
void parse_request_path(const std::string& req, std::string& path, std::string& query) {
    size_t pathStart = req.find(" ") + 1;
    size_t pathEnd = req.find(" ", pathStart);
    if (pathStart == std::string::npos || pathEnd == std::string::npos) {
        path = "/";
        query = "";
        return;
    }
    std::string fullPath = req.substr(pathStart, pathEnd - pathStart);
    size_t qPos = fullPath.find('?');
    if (qPos != std::string::npos) {
        path = fullPath.substr(0, qPos);
        query = fullPath.substr(qPos + 1);
    } else {
        path = fullPath;
        query = "";
    }
}

void serve_file(int client_socket, const std::string& filepath) {
    FILE *f = fopen(filepath.c_str(), "rb");
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
    else if (ends_with(filepath, ".svg")) content_type = "image/svg+xml";

    send_response(client_socket, "200 OK", content_type, data, fsize);
    free(data);
}

/**
 * @brief 尝试通过 AppManager 处理请求
 * @return true 如果请求已被处理
 */
static bool try_app_route(int client_socket, const std::string& path, const std::string& query) {
    auto& mgr = AppManager::instance();

    // 1. 系统级 API: 应用列表
    if (path == "/api/apps") {
        auto resp = mgr.routeRequest("GET", "/api/apps", "", "");
        if (resp) {
            send_http_response(client_socket, *resp);
            return true;
        }
    }

    // 2. 尝试匹配应用路由
    auto apps = mgr.getAllApps();
    for (auto& app : apps) {
        if (app->matchRoute(path)) {
            std::string subPath = app->stripPrefix(path);

            // API 请求 -> 由应用处理
            if (subPath.find("/api/") == 0) {
                auto resp = mgr.routeRequest("GET", path, query, "");
                if (resp) {
                    send_http_response(client_socket, *resp);
                    return true;
                }
            }

            // 静态文件请求 -> 直接从应用静态目录提供
            std::string staticDir = app->getStaticDir();
            if (!staticDir.empty()) {
                if (subPath == "/" || subPath.empty()) {
                    // 返回应用首页
                    std::string indexPage = app->getIndexPage();
                    if (!indexPage.empty()) {
                        serve_file(client_socket, indexPage);
                        return true;
                    }
                } else {
                    // 返回静态资源文件
                    std::string filePath = staticDir + subPath;
                    serve_file(client_socket, filePath);
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

void* start_web_server(void *arg) {
    (void)arg;
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

    // 设置 accept 超时，使 Web 服务器线程可以响应退出信号
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("🌐 [HTTP 服务器] 运行在 http://localhost:%d\n", PORT);

    while(g_web_running) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (!g_web_running) break;
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int read_len = read(new_socket, buffer, sizeof(buffer) - 1);
        if (read_len <= 0) {
            close(new_socket);
            continue;
        }

        std::string req(buffer);
        std::string reqPath, reqQuery;
        parse_request_path(req, reqPath, reqQuery);

        /* ============================================================
         * 路由分发 (优先级从高到低)
         * ============================================================ */

        // --- 核心系统 API ---
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

            bool submitted = false;
            if (g_cmd_queue != nullptr) {
                if (device == "ac") {
                    if (action == "cool_on")
                        submitted = g_cmd_queue->writeRegister(DEV_AC_ADDR, REG_AC_COOL_ON, 1) > 0;
                    else if (action == "cool_off")
                        submitted = g_cmd_queue->writeRegister(DEV_AC_ADDR, REG_AC_COOL_OFF, 1) > 0;
                    else if (action == "heat_on")
                        submitted = g_cmd_queue->writeRegister(DEV_AC_ADDR, REG_AC_HEAT_ON, 1) > 0;
                    else if (action == "heat_off")
                        submitted = g_cmd_queue->writeRegister(DEV_AC_ADDR, REG_AC_HEAT_OFF, 1) > 0;
                } else if (device == "humidifier") {
                    if (action == "power")
                        submitted = g_cmd_queue->writeRegister(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_POWER, val) > 0;
                    else if (action == "dehumidify")
                        submitted = g_cmd_queue->writeRegister(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_DEHUMIDIFY, val) > 0;
                    else if (action == "humidify")
                        submitted = g_cmd_queue->writeRegister(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_HUMIDIFY, val) > 0;
                    else if (action == "purify")
                        submitted = g_cmd_queue->writeRegister(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_PURIFY, val) > 0;
                    else if (action == "const_hum")
                        submitted = g_cmd_queue->writeRegister(DEV_HUMIDIFIER_ADDR, REG_HUM_CTRL_CONST_HUM, val) > 0;
                } else if (device == "purifier") {
                    if (action == "power")
                        submitted = g_cmd_queue->writeRegister(DEV_PURIFIER_ADDR, REG_PUR_CTRL_POWER, val) > 0;
                    else if (action == "run_mode")
                        submitted = g_cmd_queue->writeRegister(DEV_PURIFIER_ADDR, REG_PUR_CTRL_RUN_MODE, val) > 0;
                    else if (action == "manual")
                        submitted = g_cmd_queue->writeRegister(DEV_PURIFIER_ADDR, REG_PUR_CTRL_MANUAL, val) > 0;
                }
            }

            const char* msg = submitted ?
                "{\"status\":\"submitted\"}" : "{\"status\":\"failed\"}";
            send_response(new_socket, "200 OK", "application/json; charset=utf-8", msg, strlen(msg));

        } else if (req.find("GET /api/device/control") == 0) {
            std::string device = get_query_param(req, "device");
            std::string action = get_query_param(req, "action");
            bool ok = false;

            if (device == "water") {
                if (action == "enable") { dev_water.setOnline(true); ok = true; }
                else if (action == "disable") { dev_water.setOnline(false); ok = true; }
            } else if (device == "infrared") {
                if (action == "enable") { dev_infrared.setOnline(true); ok = true; }
                else if (action == "disable") { dev_infrared.setOnline(false); ok = true; }
            }

            const char* msg = ok ?
                "{\"status\":\"success\"}" : "{\"status\":\"failed\"}";
            send_response(new_socket, "200 OK", "application/json; charset=utf-8", msg, strlen(msg));

        } else if (req.find("GET /api/logs/list") == 0) {
            std::vector<std::string> files;
            // 扫描主日志目录下的 .log 文件
            DIR *dir = opendir("./logs");
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    std::string name = entry->d_name;
                    if (name.size() > 4 && name.substr(name.size()-4) == ".log") {
                        files.push_back(name);
                    }
                }
                closedir(dir);
            }
            // 扫描子目录 (如 logs/security/) 下的 .log 文件
            DIR *dir2 = opendir("./logs");
            if (dir2) {
                struct dirent *entry;
                while ((entry = readdir(dir2)) != NULL) {
                    std::string name = entry->d_name;
                    if (name == "." || name == "..") continue;
                    std::string subdir = "./logs/" + name;
                    struct stat st;
                    if (stat(subdir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                        DIR *sub = opendir(subdir.c_str());
                        if (sub) {
                            struct dirent *subEntry;
                            while ((subEntry = readdir(sub)) != NULL) {
                                std::string subName = subEntry->d_name;
                                if (subName.size() > 4 && subName.substr(subName.size()-4) == ".log") {
                                    files.push_back(name + "/" + subName);
                                }
                            }
                            closedir(sub);
                        }
                    }
                }
                closedir(dir2);
            }
            // 排序: gateway 日志优先 (实时), 然后按文件名降序
            std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) {
                bool aGw = (a.find("gateway") == 0);
                bool bGw = (b.find("gateway") == 0);
                if (aGw != bGw) return aGw > bGw;  // gateway 排前面
                return a > b;
            });
            std::string json = "[";
            for (size_t i = 0; i < files.size(); i++) {
                if (i > 0) json += ",";
                json += "\"" + files[i] + "\"";
            }
            json += "]";
            send_response(new_socket, "200 OK", "application/json; charset=utf-8", json.c_str(), json.size());

        } else if (req.find("GET /api/logs/latest") == 0) {
            std::string latest;
            std::string latestPath;
            // 扫描主日志目录
            DIR *dir = opendir("./logs");
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    std::string name = entry->d_name;
                    if (name.size() > 4 && name.substr(name.size()-4) == ".log") {
                        if (name > latest) {
                            latest = name;
                            latestPath = "./logs/" + name;
                        }
                    }
                }
                closedir(dir);
            }
            // 扫描子目录
            DIR *dir2 = opendir("./logs");
            if (dir2) {
                struct dirent *entry;
                while ((entry = readdir(dir2)) != NULL) {
                    std::string name = entry->d_name;
                    if (name == "." || name == "..") continue;
                    std::string subdir = "./logs/" + name;
                    struct stat st;
                    if (stat(subdir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                        DIR *sub = opendir(subdir.c_str());
                        if (sub) {
                            struct dirent *subEntry;
                            while ((subEntry = readdir(sub)) != NULL) {
                                std::string subName = subEntry->d_name;
                                if (subName.size() > 4 && subName.substr(subName.size()-4) == ".log") {
                                    std::string fullName = name + "/" + subName;
                                    if (fullName > latest) {
                                        latest = fullName;
                                        latestPath = "./logs/" + fullName;
                                    }
                                }
                            }
                            closedir(sub);
                        }
                    }
                }
                closedir(dir2);
            }
            if (!latestPath.empty()) {
                serve_file(new_socket, latestPath);
            } else {
                const char *err = "[]";
                send_response(new_socket, "200 OK", "application/json; charset=utf-8", err, strlen(err));
            }

        } else if (req.find("GET /api/logs/") == 0) {
            size_t start = 14;
            size_t end = req.find(" HTTP/");
            std::string filename = req.substr(start, end - start);
            if (filename.find("..") != std::string::npos) {
                const char *err = "403 Forbidden";
                send_response(new_socket, "403 Forbidden", "text/plain", err, strlen(err));
            } else {
                std::string path = "./logs/" + filename;
                serve_file(new_socket, path);
            }

        } else if (req.find("GET /api/devices") == 0) {
            char json[8192];
            snprintf(json, sizeof(json),
                "["
                "{\"name\":\"PM2.5传感器\",\"type\":\"cloud_pm25\",\"online\":%d,\"value\":%u,\"unit\":\"μg/m³\","
                 "\"description\":\"云测仪PM2.5细颗粒物传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"PM10传感器\",\"type\":\"cloud_pm10\",\"online\":%d,\"value\":%u,\"unit\":\"μg/m³\","
                 "\"description\":\"云测仪PM10可吸入颗粒物传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"温度传感器\",\"type\":\"cloud_temp\",\"online\":%d,\"value\":%d,\"unit\":\"°C\","
                 "\"description\":\"云测仪环境温度传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"湿度传感器\",\"type\":\"cloud_humidity\",\"online\":%d,\"value\":%u,\"unit\":\"%%\","
                 "\"description\":\"云测仪环境湿度传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"TVOC传感器\",\"type\":\"cloud_tvoc\",\"online\":%d,\"value\":%u,\"unit\":\"ppb\","
                 "\"description\":\"云测仪总挥发性有机化合物传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"甲醛传感器\",\"type\":\"cloud_ch2o\",\"online\":%d,\"value\":%u,\"unit\":\"ppb\","
                 "\"description\":\"云测仪甲醛浓度传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"臭氧传感器\",\"type\":\"cloud_o3\",\"online\":%d,\"value\":%u,\"unit\":\"ppb\","
                 "\"description\":\"云测仪臭氧浓度传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"CO2传感器\",\"type\":\"cloud_co2\",\"online\":%d,\"value\":%u,\"unit\":\"ppm\","
                 "\"description\":\"云测仪二氧化碳浓度传感器\",\"category\":\"sensor\"},"
                "{\"name\":\"烟雾探测器\",\"type\":\"smoke\",\"online\":%d,\"value\":%d,\"unit\":\"\","
                 "\"description\":\"烟雾火灾探测报警器\",\"category\":\"alarm\"},"
                "{\"name\":\"积水探测器\",\"type\":\"water\",\"online\":%d,\"value\":%d,\"unit\":\"\","
                 "\"description\":\"机房积水漏水检测传感器\",\"category\":\"alarm\"},"
                "{\"name\":\"红外探测器\",\"type\":\"infrared\",\"online\":%d,\"value\":%d,\"unit\":\"\","
                 "\"description\":\"红外入侵+雷达微波双鉴探测器\",\"category\":\"alarm\"},"
                "{\"name\":\"灯光控制器\",\"type\":\"light\",\"online\":%d,\"value\":%u,\"unit\":\"lux\","
                 "\"description\":\"智能灯光照度采集控制器\",\"category\":\"control\"},"
                "{\"name\":\"加湿器\",\"type\":\"humidifier\",\"online\":%d,\"value\":%d,\"unit\":\"\","
                 "\"description\":\"工业加湿除湿净化一体机\",\"category\":\"control\"},"
                "{\"name\":\"空调控制器\",\"type\":\"ac\",\"online\":%d,\"value\":0,\"unit\":\"\","
                 "\"description\":\"精密空调制冷制热控制器\",\"category\":\"control\"},"
                "{\"name\":\"空气净化器\",\"type\":\"purifier\",\"online\":%d,\"value\":%d,\"unit\":\"\","
                 "\"description\":\"空气净化器多模式控制器\",\"category\":\"control\"},"
                "{\"name\":\"报警设备\",\"type\":\"alarm\",\"online\":%d,\"value\":0,\"unit\":\"\","
                 "\"description\":\"声光报警联动控制器\",\"category\":\"control\"}"
                "]",
                dev_pm25.isOnline(), dev_pm25.getValue(),
                dev_pm10.isOnline(), dev_pm10.getValue(),
                dev_temperature.isOnline(), dev_temperature.getValue(),
                dev_humidity.isOnline(), dev_humidity.getValue(),
                dev_tvoc.isOnline(), dev_tvoc.getValue(),
                dev_ch2o.isOnline(), dev_ch2o.getValue(),
                dev_o3.isOnline(), dev_o3.getValue(),
                dev_co2.isOnline(), dev_co2.getValue(),
                dev_smoke.isOnline(), dev_smoke.getAlarmState(),
                dev_water.isOnline(), dev_water.getWaterState(),
                dev_infrared.isOnline(), dev_infrared.getInfraredState(),
                dev_light.isOnline(), dev_light.getIlluminance(),
                dev_humidifier.isOnline(), dev_humidifier.getPowerState(),
                dev_ac.isOnline(),
                dev_purifier.isOnline(), dev_purifier.getPowerStatus(),
                dev_alarm.isOnline()
            );
            send_response(new_socket, "200 OK", "application/json; charset=utf-8", json, strlen(json));

        } else if (req.find("GET /api/bus/stats") == 0) {
            if (g_serial_bus) {
                const BusStats &stats = g_serial_bus->getStats();
                char json[1024];
                snprintf(json, sizeof(json),
                    "{\"totalTransactions\":%lu,\"totalErrors\":%lu,"
                    "\"bytesSent\":%lu,\"bytesRecv\":%lu,"
                    "\"avgLatencyMs\":%.2f,\"maxLatencyMs\":%.2f,"
                    "\"busContentionCount\":%lu}",
                    (unsigned long)stats.totalTransactions.load(),
                    (unsigned long)stats.totalErrors.load(),
                    (unsigned long)stats.totalBytesSent.load(),
                    (unsigned long)stats.totalBytesRecv.load(),
                    stats.avgLatencyMs.load(),
                    stats.maxLatencyMs.load(),
                    (unsigned long)stats.busContentionCount.load()
                );
                send_response(new_socket, "200 OK", "application/json; charset=utf-8", json, strlen(json));
            } else {
                const char *err = "{}";
                send_response(new_socket, "200 OK", "application/json; charset=utf-8", err, strlen(err));
            }

        } else if (req.find("GET /api/cmd/stats") == 0) {
            if (g_cmd_queue) {
                const auto &stats = g_cmd_queue->getStats();
                char json[512];
                snprintf(json, sizeof(json),
                    "{\"submitted\":%lu,\"executed\":%lu,\"success\":%lu,\"failed\":%lu,"
                    "\"queueSize\":%lu,\"avgExecTimeMs\":%.1f}",
                    (unsigned long)stats.totalSubmitted.load(),
                    (unsigned long)stats.totalExecuted.load(),
                    (unsigned long)stats.totalSuccess.load(),
                    (unsigned long)stats.totalFailed.load(),
                    (unsigned long)stats.currentQueueSize.load(),
                    stats.avgExecTimeMs.load()
                );
                send_response(new_socket, "200 OK", "application/json; charset=utf-8", json, strlen(json));
            } else {
                const char *err = "{}";
                send_response(new_socket, "200 OK", "application/json; charset=utf-8", err, strlen(err));
            }

        // --- 应用路由 (由 AppManager 分发) ---
        } else if (try_app_route(new_socket, reqPath, reqQuery)) {
            // 已由应用处理, 什么都不做

        // --- 静态资源 (Vue UI) ---
        } else if (reqPath.find("/assets/") == 0) {
            size_t start = req.find("/assets/");
            size_t end = req.find(" HTTP/");
            if (start != std::string::npos && end != std::string::npos) {
                std::string path = "public" + req.substr(start, end - start);
                serve_file(new_socket, path);
            }

        // --- 默认: 返回主仪表盘 ---
        } else {
            serve_file(new_socket, "dashboard.html");
        }

        close(new_socket);
    }

    close(server_fd);
    printf("🌐 [HTTP 服务器] 已停止\n");
    return NULL;
}