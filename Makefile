CC = gcc
CFLAGS = -Wall -I. -I./devices -I./core
LDFLAGS = -lpthread
TARGET = app_gateway
SRCS = main.c core/modbus_core.c  core/web_server.c devices/dev_cloud.c devices/dev_smoke.c devices/dev_water.c devices/dev_infrared.c devices/dev_light.c
OBJS = $(SRCS:.c=.o)
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
