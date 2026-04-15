#include "sensor_data.h"
#include <string.h>

SensorData g_sensor_data;

void init_sensor_data() {
    memset(&g_sensor_data, 0, sizeof(SensorData));
    pthread_mutex_init(&g_sensor_data.lock, NULL);
}