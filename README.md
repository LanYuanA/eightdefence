# 八防综合监控网关平台 (Multi-Defense Sensor Gateway)

基于 C++ 开发的工程化、面向对象、事件驱动的 Modbus RTU 传感器网关及全栈监控面板。系统通过 RS485 串口定时轮询各类环境与安防传感器，对报文进行 CRC 校验和业务解析，同时在后台启动独立线程的 C++ HTTP Server，为 Vue 3 + Vite 构建的赛博朋克风格大屏提供静态资源托管及 RESTful API 数据分发，并支持对受控设备的远程指令透传。

## 特性

- **面向对象设备抽象**：基于 `DeviceBase` 基类继承体系，每个传感器与控制设备拥有独立的读写及 Modbus 命令组帧能力。
- **全自动 CRC-16 校验**：透明计算并拼接 RS485 总线数据帧的校验码，上层无需关心底层通信细节。
- **表驱动任务调度 (Table-Driven)**：设备通过 `DeviceTask` 结构自描述轮询任务，新增设备即插即用。
- **在线状态监控与容错**：网关实时追踪各设备通信状态，连续失败自动标记离线并在 API 响应中携带告警。
- **云测仪连续读/单独读**：支持一次 Modbus 读取 8 个连续寄存器（减少 85% 总线占用），可通过 API 运行时切换。
- **多应用架构**：安防、环境监测、消防三个独立应用，各自拥有独立的前端页面和 API。
- **原子服务联动**：风险评估自动驱动声光报警、净化器、温湿度调控等原子服务，设备离线时自动跳过指令。
- **现代前端工程化**：Vue 3 + Vite + TypeScript + Tailwind CSS，集成 Element Plus 与 Vue-ECharts 实时图表。
- **双向控制接口**：通过 `/api/control` 提供下行控制信道，支持开关机、模式切换等操作。
- **登录鉴权**：前端集成登录页面与路由守卫，基于 localStorage 的会话管理。

## 支持的设备

| 设备 | 地址 | 类型 | 说明 |
|------|------|------|------|
| 室内空气质量变送器 (SD123-E60V2) | `0x30` | 传感器 | CH2O / PM2.5 / TVOC / CO2 / 温度 / 湿度 / PM10 |
| 烟雾报警器 | `0x70` | 传感器 | 烟雾检测报警 |
| 水浸传感器 | `0x90` | 传感器 | 水浸检测 |
| 红外探测器 | `0x40` | 传感器 | 红外 + 雷达双鉴探测 |
| 光感传感器 | `0x50` | 传感器 | 环境光照度检测 |
| 恒湿净化一体机 | `0x20` | 控制设备 | 开关机 / 除湿 / 加湿 / 净化 / 恒湿模式控制 |
| 空调集中控制器 | `0x60` | 控制设备 | 制冷 / 制热开关控制 |
| 霉菌空气净化机 | `0x10` | 控制设备 | 开关机 / 运行模式 / 手动模式控制 |
| 报警装置 (驱鼠器) | `0x80` | 控制设备 | 继电器吸合/断开控制 |

## 应用模块

| 应用 | 路由 | 说明 |
|------|------|------|
| 主控台 | `/` | 实时传感器数据面板 + ECharts 趋势图 + 设备控制 |
| 安防系统 | `/security` | 水浸/入侵/有害气体检测 + 原子服务联动 |
| 环境监测 | `/environment` | 温湿度/PM2.5/PM10/TVOC/CH2O/CO2 环境监测 |
| 消防系统 | `/fire` | 烟雾/温度火灾检测 + 灭火/疏散联动 |
| 设备管理 | `/devices` | 所有设备在线状态与历史数据 |

## 项目结构

```text
├── application/
│   ├── app_base.hpp               # 应用基类 (多应用框架核心)
│   ├── app_manager.hpp            # 应用管理器 (单例, 路由分发)
│   ├── web_server.cpp/h           # C++ HTTP Server
│   └── apps/
│       ├── security/              # 安防应用
│       │   ├── app_security.hpp
│       │   └── app_security.cpp
│       ├── environment/           # 环境监测应用
│       │   ├── app_environment.hpp
│       │   └── app_environment.cpp
│       └── fire_fighting/         # 消防应用
│           ├── app_fire_fighting.hpp
│           └── app_fire_fighting.cpp
├── core/
│   ├── device_config.h            # 全设备 Modbus 寄存器地址定义
│   ├── global_devices.hpp         # 全局设备实例声明 (extern)
│   ├── main.cpp                   # 主入口: 设备初始化、服务创建、应用注册
│   ├── modbus_core.c/h            # RS485 串口通信底层与 CRC-16 校验
│   ├── serial_bus.cpp/h           # 线程安全串口总线
│   ├── async_bus.cpp/h            # 异步总线 (写优先队列)
│   ├── polling_manager.cpp/h      # 多线程轮询管理器
│   ├── command_queue.cpp/h        # 命令队列
│   ├── logger.cpp/h               # 日志系统
│   └── app_logger.cpp/h           # 应用独立日志器
├── devices/
│   ├── device_base.hpp            # 设备抽象基类 (DeviceBase / DeviceTask / DeviceStatus)
│   ├── dev_common.c/h             # 公共工具函数
│   ├── dev_cloud_sensors.cpp/hpp  # 云测仪 7 路传感器 (支持连续读/单独读)
│   ├── dev_smoke.cpp/hpp          # 烟雾报警器
│   ├── dev_water.cpp/hpp          # 水浸传感器
│   ├── dev_infrared.cpp/hpp       # 红外+雷达双鉴探测器
│   ├── dev_light.cpp/hpp          # 光感传感器
│   ├── dev_humidifier.cpp/hpp     # 恒湿净化一体机
│   ├── dev_air_conditioner.cpp/hpp # 空调集中控制器
│   ├── dev_air_purifier.cpp/hpp   # 霉菌空气净化机
│   └── dev_alarm_device.cpp/hpp   # 报警装置
├── service/
│   ├── modbus_service.cpp/hpp     # Modbus 通信服务
│   ├── parse_service.cpp/hpp      # 报文解析服务
│   └── atomic/                    # 原子服务 (风险联动)
│       ├── svc_sound_light_alarm.cpp/hpp  # 声光报警
│       ├── svc_drainage.cpp/hpp           # 排水控制
│       ├── svc_temp_humidity_control.cpp/hpp  # 温湿度调控
│       ├── svc_gas_response.cpp/hpp       # 有害气体处理
│       ├── svc_command_center.cpp/hpp     # 指挥中心报警
│       ├── svc_air_quality_alert.cpp/hpp  # 空气质量告警
│       ├── svc_ventilation.cpp/hpp        # 通风换气
│       ├── svc_fire_suppression.cpp/hpp   # 灭火联动
│       └── svc_evacuation.cpp/hpp         # 疏散引导
├── ui/                            # Vue 3 前端工程
│   ├── src/
│   │   ├── views/
│   │   │   ├── Login.vue          # 登录页面
│   │   │   ├── Dashboard.vue      # 主控台 (传感器面板 + ECharts 图表)
│   │   │   ├── Security.vue       # 安防系统页面
│   │   │   ├── Environment.vue    # 环境监测页面
│   │   │   ├── FireFighting.vue   # 消防系统页面
│   │   │   ├── Devices.vue        # 设备管理页面
│   │   │   └── LogViewer.vue      # 日志查看器
│   │   ├── router/index.ts        # Vue Router 路由配置
│   │   └── main.ts
│   ├── vite.config.ts             # Vite 配置 (含 API 代理)
│   └── package.json
├── public/                        # 构建产物部署目录
├── doc/                           # 设计文档
├── Makefile                       # 构建脚本
└── dashboard.html                 # 生产环境入口 HTML
```

## 编译与运行

### 1. 环境准备

- `g++` (支持 C++17)
- `make`
- `Node.js` (v18+) 与 npm
- RS485 硬件串口 (默认 `/dev/ttyS9`)

### 2. 一键构建

```bash
make          # 编译前端 + C++ 后端
```

### 3. 运行

```bash
# 生产模式: 一个端口提供所有服务
./app_gateway

# 开发模式 (推荐): C++ 后端 + Vue 开发服务器 (带热更新)
./app_gateway           # 终端1: 后端 (端口 8080)
cd ui && npm run dev    # 终端2: 前端 (端口 5173, 自动代理 API)
```

### 4. 访问

- 生产: `http://localhost:8080`
- 开发: `http://localhost:5173`

## API 接口

### 系统级

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/api/data` | 所有传感器数据与在线状态 |
| GET | `/api/control?device=xxx&action=yyy&val=zzz` | 下行控制指令 |
| GET | `/api/apps` | 已注册的应用列表 |

### 云测仪连续读控制

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/api/control?device=cloud&action=batch&val=1` | 开启连续读 (默认) |
| GET | `/api/control?device=cloud&action=batch&val=0` | 切换为单独读 |

### 应用级 (安防/环境/消防)

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/security/api/status` | 安防系统状态 |
| GET | `/security/api/sensors` | 安防传感器数据 |
| GET | `/security/api/control?action=simulate&target=xxx` | 模拟异常 |
| GET | `/security/api/control?action=reset&target=xxx` | 恢复正常 |
| GET | `/security/api/logs` | 安防事件日志 |
| GET | `/environment/api/status` | 环境监测状态 |
| GET | `/environment/api/sensors` | 环境传感器数据 |
| GET | `/environment/api/control?action=simulate&target=xxx` | 模拟异常 |
| GET | `/environment/api/logs` | 环境事件日志 |
| GET | `/fire/api/status` | 消防系统状态 |
| GET | `/fire/api/sensors` | 消防传感器数据 |
| GET | `/fire/api/control?action=simulate&target=xxx` | 模拟火灾/烟雾 |
| GET | `/fire/api/logs` | 消防事件日志 |

## 云测仪寄存器 (SD123-E60V2)

寄存器地址连续排列 0x0000~0x0007，支持一次读取 8 个：

| 地址 | 数据 | 类型 | 单位 |
|------|------|------|------|
| 0x0000 | 甲醛 CH2O | 16位无符号 | ppb |
| 0x0001 | PM2.5 | 16位无符号 | μg/m³ |
| 0x0002 | TVOC | 16位无符号 | μg/m³ |
| 0x0003 | CO2 | 16位无符号 | ppm |
| 0x0004 | 温度 | 16位有符号 | 0.1°C |
| 0x0005 | 湿度 | 16位无符号 | 0.1% |
| 0x0006 | PM1.0 | 16位无符号 | μg/m³ |
| 0x0007 | PM10 | 16位无符号 | μg/m³ |

## 原子服务联动

| 服务 | 触发条件 | 执行动作 |
|------|----------|----------|
| 声光报警 | 任意高风险 | 驱动报警器蜂鸣 |
| 排水控制 | 水浸高风险 | 启动排水泵 |
| 温湿度调控 | 温度/湿度超标 | 空调制冷/制热 + 加湿/除湿 |
| 有害气体处理 | TVOC/CH2O/CO2 超标 | 开启净化器 |
| 空气质量告警 | PM2.5/PM10 超标 | 开启净化器 |
| 通风换气 | 气体综合超标 | 启动通风 (模拟) |
| 指挥中心报警 | 任意高风险 | 记录告警事件 |
| 灭火联动 | 烟雾检测到 | 启动灭火装置 (模拟) |
| 疏散引导 | 火灾确认 | 启动疏散广播 (模拟) |

**设备离线保护**：所有原子服务在发送指令前检查 `isOnline()`，设备离线时自动跳过，不占用总线时间。

## 拓展新设备

1. 在 `devices/` 下新建 `.cpp/.hpp`，继承 `DeviceBase`。
2. 实现 `init()`、`getTasks()`、`getName()` 接口。
3. 在 `core/device_config.h` 中定义寄存器地址。
4. 在 `core/global_devices.hpp` 中声明全局实例。
5. 在 `core/main.cpp` 中实例化并收集任务。
6. 在 `application/web_server.cpp` 中添加 API 字段。

## 拓展新应用

1. 在 `application/apps/<name>/` 下创建 `app_<name>.hpp/.cpp`。
2. 继承 `AppBase`，在构造函数中 `registerApi()` 注册路由。
3. 实现 `init()`、`start()`、`stop()` 生命周期。
4. 在 `core/main.cpp` 中创建实例并 `registerApp()`。
5. 在 `ui/src/views/` 中创建 Vue 页面。
6. 在 `ui/src/router/index.ts` 中添加路由。

## 许可证

本项目仅供学习与研究使用。
