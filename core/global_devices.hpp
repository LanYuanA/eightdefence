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
#include "devices/dev_stepper_motor.hpp"
#include "service/modbus_service.hpp"
#include "serial_bus.hpp"
#include "polling_manager.hpp"
#include "command_queue.hpp"

extern DevCloudPm25        dev_pm25;
extern DevCloudPm10        dev_pm10;
extern DevCloudHumidity    dev_humidity;
extern DevCloudTemperature dev_temperature;
extern DevCloudTvoc        dev_tvoc;
extern DevCloudCh2o        dev_ch2o;
extern DevCloudCo2         dev_co2;
extern DevCloudBatch       dev_cloud_batch;

extern DevSmoke            dev_smoke;
extern DevWater            dev_water;
extern DevInfrared         dev_infrared;
extern DevLight            dev_light;
extern DevHumidifier       dev_humidifier;
extern DevAirConditioner   dev_ac;
extern DevAirPurifier      dev_purifier;
extern DevAlarmDevice      dev_alarm;
extern DevStepperMotor     dev_stepper_cabin;     // 舱门电机 0x01
extern DevStepperMotor     dev_stepper_sprinkler;  // 水淋电机 0x02
extern DevStepperMotor     dev_stepper_exhaust;   // 排烟风机 0x03

extern ModbusService*      g_modbus;
extern SerialBus*          g_serial_bus;
extern CommandQueue*       g_cmd_queue;
extern PollingManager*     g_poller;

// 全局退出标志 (Web 服务器和主线程共享)
extern volatile bool       g_web_running;

#endif // GLOBAL_DEVICES_HPP
