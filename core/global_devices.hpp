#ifndef GLOBAL_DEVICES_HPP
#define GLOBAL_DEVICES_HPP

#include "devices/dev_cloud_sensors.hpp"
#include "devices/dev_smoke.hpp"
#include "devices/dev_water.hpp"
#include "devices/dev_infrared.hpp"
#include "devices/dev_light.hpp"
#include "devices/dev_humidifier.hpp"
#include "devices/dev_air_conditioner.hpp"
#include "devices/dev_air_purifier.hpp"
#include "devices/dev_alarm_device.hpp"
#include "service/modbus_service.hpp"

extern DevCloudPm25        dev_pm25;
extern DevCloudPm10        dev_pm10;
extern DevCloudHumidity    dev_humidity;
extern DevCloudTemperature dev_temperature;
extern DevCloudTvoc        dev_tvoc;
extern DevCloudCh2o        dev_ch2o;
extern DevCloudO3          dev_o3;
extern DevCloudCo2         dev_co2;

extern DevSmoke            dev_smoke;
extern DevWater            dev_water;
extern DevInfrared         dev_infrared;
extern DevLight            dev_light;
extern DevHumidifier       dev_humidifier;
extern DevAirConditioner   dev_ac;
extern DevAirPurifier      dev_purifier;
extern DevAlarmDevice      dev_alarm;

extern ModbusService*      g_modbus;

#endif // GLOBAL_DEVICES_HPP
