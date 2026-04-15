#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#define REG_READ 0x03
// 01. 室内空气质量变送器 (云测仪) 地址30 指令：30 03  REG_CLOUD_* 0001
#define DEV_CLOUD_ADDR     0x30
#define REG_CLOUD_PM25     0x0000
#define REG_CLOUD_PM10     0x0001
#define REG_CLOUD_HUMIDITY 0x0002
#define REG_CLOUD_TEMP     0x0003
#define REG_CLOUD_TVOC     0x0007
#define REG_CLOUD_CH2O     0x0009
#define REG_CLOUD_O3       0x000A
#define REG_CLOUD_CO2      0x000F
#define REG_CLOUD_DATA     0x0001
// 02. 烟雾报警器 地址70 指令70 03 00 01 00 0A
#define DEV_SMOKE_ADDR     0x70
#define REG_SMOKE_ALARM    0x0001
#define REG_SMOKE_DATA     0x000A
// 03. 驱鼠器 地址80 
//800100000001 读取继电器
//80050000FF00 继电器吸合
//800500000000 继电器断开
#define DEV_MOUSE_ADDR     0x80
#define REG_MOUSE_READ     0x01//查询操作
#define REG_MOUSE_CTRL     0x05//控制操作
#define REG_MOUSE_ADDR     0x0000
#define REG_MOUSE_STATUS   0x0001//表示读取继电器
#define REG_MOUSE_OPEN     0xFF00//继电器吸合
#define REG_MOUSE_CLOSE     0x0000//继电器断开
// 04. 水浸传感器 地址90 指令 90 03 00 10 0001
#define DEV_WATER_ADDR     0x90
#define REG_WATER_STATUS   0x0010
#define REG_WATER_DATA     0x0001
// 05. 红外探测器
#define DEV_INFRARED_ADDR  0x40
#define REG_INFRARED_STATE 0x0006
#define REG_RADAR_STATE    0x0007
#define REG_INFRARED_DATA  0x0001
// 06. 弱光检测传感器 (光感) 50 03 00 00 00 01
#define DEV_LIGHT_ADDR     0x50
#define REG_LIGHT_ILLUMIN  0x0000
#define REG_LIGHT_DATA     0x0001
#endif // DEVICE_CONFIG_H
