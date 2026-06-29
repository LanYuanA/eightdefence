#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

/* ============================================================
 * 功能码定义
 * ============================================================ */
#define REG_READ           0x03  /* 读保持寄存器 */
#define REG_WRITE_SINGLE   0x06  /* 写单个寄存器 */
#define REG_WRITE_MULTI    0x10  /* 写多个寄存器 */
#define REG_COIL_READ      0x01  /* 读线圈状态 */
#define REG_COIL_WRITE     0x05  /* 写单个线圈 */

/* ============================================================
 * 01. 室内空气质量变送器 (云测仪) SD123-E60V2  地址: 0x30
 *     寄存器 0x0000~0x0007 连续排列, 一次读取8个寄存器
 * ============================================================ */
#define DEV_CLOUD_ADDR     0x30
#define REG_CLOUD_CH2O     0x0000  /* 甲醛 (ppb, 16-bit unsigned) */
#define REG_CLOUD_PM25     0x0001  /* PM2.5 (ug/m3, 16-bit unsigned) */
#define REG_CLOUD_TVOC     0x0002  /* TVOC (ug/m3, 16-bit unsigned) */
#define REG_CLOUD_CO2      0x0003  /* CO2 (ppm, 16-bit unsigned) */
#define REG_CLOUD_TEMP     0x0004  /* 温度 (0.1C, 16-bit signed) */
#define REG_CLOUD_HUMIDITY 0x0005  /* 湿度 (0.1%, 16-bit unsigned) */
#define REG_CLOUD_PM10     0x0007  /* PM10 (ug/m3, 16-bit unsigned) */
#define REG_CLOUD_DATA     0x0001  /* 读取寄存器数量 (单次读1个) */

/* ============================================================
 * 02. 烟雾报警器  地址: 0x70
 * ============================================================ */
#define DEV_SMOKE_ADDR     0x70
#define REG_SMOKE_ALARM    0x0001
#define REG_SMOKE_DATA     0x000A

/* ============================================================
 * 03. 恒湿净化一体机  地址: 0x20
 * ============================================================ */
#define DEV_HUMIDIFIER_ADDR        0x20
/* --- 数据读取寄存器 (功能码 0x03, 寄存器数量 0x0005) --- */
#define REG_HUM_CO2                0x1001
#define REG_HUM_CH2O               0x1002
#define REG_HUM_TVOC               0x1003
#define REG_HUM_PM25               0x1004
#define REG_HUM_PM10               0x1005
#define REG_HUM_TEMPERATURE        0x1006
#define REG_HUM_HUMIDITY           0x1007
#define REG_HUM_DEFROST_TEMP       0x1008
#define REG_HUM_AIR_QUALITY        0x1009
#define REG_HUM_POWER_STATE        0x100A
#define REG_HUM_DEHUMIDIFY_STATE   0x100B
#define REG_HUM_HUMIDIFY_STATE     0x100C
#define REG_HUM_PURIFY_STATE       0x100D
#define REG_HUM_DRAINAGE_STATE     0x100E
#define REG_HUM_CONST_HUM_STATE    0x100F
#define REG_HUM_FROST_STATE        0x1010
#define REG_HUM_FULL_WATER_STATE   0x1011
#define REG_HUM_WATER_FULL_SIG     0x1012
#define REG_HUM_WATER_SHORT_SIG    0x1013
#define REG_HUM_FAN_OUTPUT_STATE   0x1017
#define REG_HUM_DEHUM_FAN_STATE    0x1018
#define REG_HUM_HUMIDIFY_FAN_STATE 0x1019
#define REG_HUM_PURIFY_FAN_STATE   0x101A
#define REG_HUM_DEHUM_OUTPUT_STATE 0x101B
#define REG_HUM_PURIFY_OUTPUT_STATE 0x101C
#define REG_HUM_HUMIDIFY_OUTPUT_STATE 0x101D
#define REG_HUM_DRAINAGE_STATE2    0x101E
#define REG_HUM_FAULT_STATE        0x101F
#define REG_HUM_READ_COUNT         0x0005  /* 读取寄存器数量 */
/* --- 控制寄存器 (功能码 0x06, 值 0x0001) --- */
#define REG_HUM_CTRL_CONST_HUM     0x1043  /* 恒湿开关 */
#define REG_HUM_CTRL_POWER         0x1045  /* 开关机控制 */
#define REG_HUM_CTRL_DEHUMIDIFY    0x1046  /* 除湿开关 */
#define REG_HUM_CTRL_HUMIDIFY      0x1047  /* 加湿开关 */
#define REG_HUM_CTRL_PURIFY        0x1048  /* 净化开关 */

/* ============================================================
 * 04. 报警装置 (驱鼠器)  地址: 0x80
 * ============================================================ */
#define DEV_ALARM_ADDR     0x80
#define REG_ALARM_READ     0x01   /* 查询操作 (功能码) */
#define REG_ALARM_CTRL     0x05   /* 控制操作 (功能码) */
#define REG_ALARM_COIL     0x0000 /* 线圈地址 */
#define REG_ALARM_DATA     0x0001 /* 读取线圈数量 */
#define REG_ALARM_ON       0xFF00 /* 继电器吸合 */
#define REG_ALARM_OFF      0x0000 /* 继电器断开 */

/* ============================================================
 * 05. 水浸传感器  地址: 0x90
 * ============================================================ */
#define DEV_WATER_ADDR     0x90
#define REG_WATER_STATUS   0x0010
#define REG_WATER_DATA     0x0001

/* ============================================================
 * 06. 红外探测器  地址: 0x40
 * ============================================================ */
#define DEV_INFRARED_ADDR  0x40
#define REG_INFRARED_STATE 0x0006
#define REG_RADAR_STATE    0x0007
#define REG_INFRARED_DATA  0x0001

/* ============================================================
 * 07. 空调集中控制器  地址: 0x60
 * ============================================================ */
#define DEV_AC_ADDR        0x60
#define REG_AC_COOL_ON     0x0000  /* 制冷开机 */
#define REG_AC_COOL_OFF    0x0001  /* 制冷关机 */
#define REG_AC_HEAT_ON     0x0002  /* 制热开机 */
#define REG_AC_HEAT_OFF    0x0003  /* 制热关机 */
#define REG_AC_DATA        0x0007  /* 读取寄存器数量 */

/* ============================================================
 * 08. 霉菌空气净化机  地址: 0x10
 * ============================================================ */
#define DEV_PURIFIER_ADDR          0x10
/* --- 数据读取寄存器 (功能码 0x03, 寄存器数量 0x0001) --- */
#define REG_PUR_RUN_MODE           0x0003  /* 运行模式 */
#define REG_PUR_CO2                0x1001
#define REG_PUR_CH2O               0x1002
#define REG_PUR_TVOC               0x1003
#define REG_PUR_PM25               0x1004
#define REG_PUR_PM10               0x1005
#define REG_PUR_TEMPERATURE        0x1006
#define REG_PUR_HUMIDITY           0x1007
#define REG_PUR_AIR_QUALITY_LEVEL  0x1009
#define REG_PUR_POWER_STATUS       0x100A
#define REG_PUR_READ_COUNT         0x0001  /* 读取寄存器数量 */
/* --- 控制寄存器 (功能码 0x06) --- */
#define REG_PUR_CTRL_RUN_MODE      0x0003  /* 运行模式设定 */
#define REG_PUR_CTRL_POWER         0x0100  /* 开关机状态设定 */
#define REG_PUR_CTRL_MANUAL        0x0008  /* 手动模式选择 */
#define REG_PUR_CTRL_TIMING        0x0019  /* 定时模式选择 */

/* ============================================================
 * 09. 弱光检测传感器 (光感)  地址: 0x50
 * ============================================================ */
#define DEV_LIGHT_ADDR     0x50
#define REG_LIGHT_ILLUMIN  0x0000
#define REG_LIGHT_DATA     0x0001

#endif /* DEVICE_CONFIG_H */