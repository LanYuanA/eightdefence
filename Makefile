CC = gcc
CXX = g++
CFLAGS = -Wall -I. -I./devices -I./core -I./application -I./service
CXXFLAGS = -Wall -std=c++17 -I. -I./devices -I./core -I./application -I./service
LDFLAGS = -lpthread

TARGET = app_gateway

# C 源文件 (保留不变的底层模块)
C_SRCS = core/modbus_core.c \
         devices/dev_common.c
C_OBJS = $(C_SRCS:.c=.o)

# C++ 源文件 (新抽象层 + 主程序)
CXX_SRCS = service/modbus_service.cpp service/parse_service.cpp \
           service/atomic/svc_sound_light_alarm.cpp service/atomic/svc_drainage.cpp \
           service/atomic/svc_temp_humidity_control.cpp service/atomic/svc_gas_response.cpp \
           service/atomic/svc_command_center.cpp \
           service/atomic/svc_air_quality_alert.cpp service/atomic/svc_ventilation.cpp \
           service/atomic/svc_fire_cabin.cpp service/atomic/svc_fire_sprinkler.cpp service/atomic/svc_fire_fan.cpp \
           devices/dev_smoke.cpp devices/dev_water.cpp \
           devices/dev_infrared.cpp devices/dev_light.cpp \
           devices/dev_cloud_sensors.cpp devices/dev_humidifier.cpp \
           devices/dev_air_conditioner.cpp devices/dev_air_purifier.cpp \
           devices/dev_alarm_device.cpp \
           devices/dev_stepper_motor.cpp \
           application/web_server.cpp application/app_base.cpp \
           application/apps/security/app_security.cpp \
           application/apps/environment/app_environment.cpp \
           application/apps/fire_fighting/app_fire_fighting.cpp \
           core/serial_bus.cpp core/async_bus.cpp core/polling_manager.cpp \
           core/logger.cpp core/app_logger.cpp core/command_queue.cpp \
           core/main.cpp
CXX_OBJS = $(CXX_SRCS:.cpp=.o)

OBJS = $(C_OBJS) $(CXX_OBJS)

# 前端编译: npm run build + 拷贝到 public + 更新 dashboard.html
# 强制每次都检查编译（Vite使用哈希文件名，Make无法追踪依赖变化）
.PHONY: frontend
frontend:
	cd ui && npm run build
	cp -f ui/dist/assets/*.js public/assets/
	cp -f ui/dist/assets/*.css public/assets/
	NEW_JS=$$(ls ui/dist/assets/*.js | head -1 | xargs basename); \
	NEW_CSS=$$(ls ui/dist/assets/*.css | head -1 | xargs basename); \
	sed -i "s|src=\"/assets/[^\"]*\.js\"|src=\"/assets/$$NEW_JS\"|" dashboard.html; \
	sed -i "s|href=\"/assets/[^\"]*\.css\"|href=\"/assets/$$NEW_CSS\"|" dashboard.html
	@echo "✅ 前端已编译并部署到C++ public目录"

all: frontend $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(C_OBJS) $(CXX_OBJS) $(TARGET)

clean-all: clean
	rm -rf ui/dist ui/node_modules public/assets/*.js public/assets/*.css