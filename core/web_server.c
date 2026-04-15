#include "web_server.h"
#include "sensor_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define PORT 8080

void send_response(int client_socket, const char *header, const char *content_type, const char *body, size_t body_len) {
    char headers[512];
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

void* start_web_server(void *arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

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

        read(new_socket, buffer, 1024);
        
        if (strncmp(buffer, "GET /api/data", 13) == 0) {
            // 返回 JSON 数据
            char json[1024];
            pthread_mutex_lock(&g_sensor_data.lock);
            snprintf(json, sizeof(json),
                "{"
                "\"temp\": %.1f, \"hum\": %.1f, \"pm25\": %d, \"pm10\": %d, \"ch2o\": %d, "
                "\"co2\": %d, \"o3\": %d, \"tvoc\": %d, "
                "\"smoke\": %d, \"smoke_temp\": %d, "
                "\"water\": %d, "
                "\"ir\": %d, \"radar\": %d, "
                "\"lux\": %d"
                "}",
                g_sensor_data.temp, g_sensor_data.hum, g_sensor_data.pm25, g_sensor_data.pm10, g_sensor_data.ch2o,
                g_sensor_data.co2, g_sensor_data.o3, g_sensor_data.tvoc,
                g_sensor_data.smoke_status, g_sensor_data.smoke_temp_thresh,
                g_sensor_data.water_status,
                g_sensor_data.ir_state, g_sensor_data.radar_state,
                g_sensor_data.lux
            );
            pthread_mutex_unlock(&g_sensor_data.lock);
            
            send_response(new_socket, "200 OK", "application/json; charset=utf-8", json, strlen(json));
            
        } else {
            // 处理根路径 / 或者 /dashboard.html (读取并发送 HTML 文件)
            FILE *f = fopen("dashboard.html", "r");
            if (f) {
                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                fseek(f, 0, SEEK_SET);

                char *html = malloc(fsize + 1);
                fread(html, 1, fsize, f);
                fclose(f);
                html[fsize] = 0;

                send_response(new_socket, "200 OK", "text/html; charset=utf-8", html, fsize);
                free(html);
            } else {
                const char *err = "Dashboard HTML not found.";
                send_response(new_socket, "404 Not Found", "text/plain", err, strlen(err));
            }
        }
        close(new_socket);
    }
    return NULL;
}