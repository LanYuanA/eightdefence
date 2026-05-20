CC = gcc
CFLAGS = -Wall -I. -I./devices -I./core -I./application -I./service
LDFLAGS = -lpthread

TARGET = app_gateway
SRCS = core/main.c core/modbus_core.c application/web_server.c \
       service/modbus_service.c service/parse_service.c \
       devices/dev_common.c devices/dev_cloud.c devices/dev_smoke.c \
       devices/dev_water.c devices/dev_infrared.c devices/dev_light.c \
       devices/dev_humidifier.c devices/dev_air_conditioner.c \
       devices/dev_air_purifier.c devices/dev_alarm_device.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)