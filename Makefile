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
           devices/dev_smoke.cpp devices/dev_water.cpp \
           devices/dev_infrared.cpp devices/dev_light.cpp \
           devices/dev_cloud_sensors.cpp devices/dev_humidifier.cpp \
           devices/dev_air_conditioner.cpp devices/dev_air_purifier.cpp \
           devices/dev_alarm_device.cpp \
           application/web_server.cpp application/app_base.cpp \
           application/apps/security/app_security.cpp \
           core/serial_bus.cpp core/polling_manager.cpp \
           core/logger.cpp core/app_logger.cpp core/command_queue.cpp \
           core/main.cpp
CXX_OBJS = $(CXX_SRCS:.cpp=.o)

OBJS = $(C_OBJS) $(CXX_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(C_OBJS) $(CXX_OBJS) $(TARGET)