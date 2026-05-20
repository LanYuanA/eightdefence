#include "dev_common.h"
#include <string.h>

void dev_status_init(DeviceStatus *st) {
    st->online     = 1;
    st->fail_count = 0;
    pthread_mutex_init(&st->lock, NULL);
}

void dev_status_on_success(DeviceStatus *st) {
    pthread_mutex_lock(&st->lock);
    st->fail_count = 0;
    st->online     = 1;
    pthread_mutex_unlock(&st->lock);
}

int dev_status_on_failure(DeviceStatus *st, int threshold) {
    int went_offline = 0;
    pthread_mutex_lock(&st->lock);
    st->fail_count++;
    if (st->fail_count >= threshold) {
        st->online = 0;
        went_offline = 1;
    }
    pthread_mutex_unlock(&st->lock);
    return went_offline;
}

int dev_status_is_online(DeviceStatus *st) {
    pthread_mutex_lock(&st->lock);
    int val = st->online;
    pthread_mutex_unlock(&st->lock);
    return val;
}

void env_data_init(EnvData *data) {
    memset(data, 0, sizeof(EnvData));
    pthread_mutex_init(&data->lock, NULL);
}