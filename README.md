# 八防综合监控网关平台 (Multi-Defense Sensor Gateway)

基于 C++ 开发的工程化、面向对象、事件驱动的 Modbus RTU 传感器网关及全栈监控面板。系统通过 RS485 串口定时轮询各类环境与安防传感器，对报文进行 CRC 校验和业务解析，同时在后台启动独立线程的 C++ HTTP Server，为 Vue 3 + Vite 构建的赛博朋克风格大屏提供静态资源托管及 RESTful API 数据分发，并支持对受控设备的远程指令透传。

## 特性

- **面向对象设备抽象**：基于 `DeviceBase` 基类继承体系，每个传感器与控制设备（空调、恒湿机、净化器等）拥有独立的读写及 Modbus 命令组帧能力。
- **全自动 CRC-16 校验**：透明计算并拼接 RS485 总线数据帧的校验码，上层无需关心底层通信细节。
- **表驱动任务调度 (Table-Driven)**：设备通过 `DeviceTask` 结构自描述轮询任务，无需臃肿的 `switch-case`，新增设备即插即用，与 Web Server、解析模块完全解耦。
- **在线状态监控与容错**：网关实时追踪各设备通信状态，连续 3 次失败自动标记离线并在 API 响应中携带告警。
- **现代前端工程化**：Vue 3 + Vite + TypeScript + Tailwind CSS 构建赛博朋克深色面板，集成 Element Plus 组件库与 Vue-ECharts 实时图表。
- **双向控制接口**：针对空调、恒湿一体机、霉菌净化机等设备，通过 `/api/control` 提供下行控制信道，支持开关机、模式切换、参数设定等操作。
- **登录鉴权**：前端集成登录页面与路由守卫，基于 localStorage 的会话管理。

## 支持的设备

| 设备 | 地址 | 类型 | 说明 |
|------|------|------|------|
| 室内空气质量变送器 (云测仪) | `0x30` | 传感器 | PM2.5 / PM10 / 温度 / 湿度 / TVOC / CH2O / O3 / CO2 |
| 烟雾报警器 | `0x70` | 传感器 | 烟雾检测报警 |
| 水浸传感器 | `0x90` | 传感器 | 水浸检测 |
| 红外探测器 | `0x40` | 传感器 | 红外 + 雷达双鉴探测 |
| 光感传感器 | `0x50` | 传感器 | 环境光照度检测 |
| 恒湿净化一体机 | `0x20` | 控制设备 | 开关机 / 除湿 / 加湿 / 净化 / 恒湿模式控制 |
| 空调集中控制器 | `0x60` | 控制设备 | 制冷 / 制热开关控制 |
| 霉菌空气净化机 | `0x10` | 控制设备 | 开关机 / 运行模式 / 手动模式控制 |
| 报警装置 (驱鼠器) | `0x80` | 控制设备 | 继电器吸合/断开控制 |

## 项目结构

```text
├── application/
│   ├── web_server.cpp           # C++ HTTP Server：静态资源托管 + RESTful API
│   └── web_server.h
├── core/
│   ├── device_config.h          # 全设备 Modbus 寄存器地址与功能码定义
│   ├── global_devices.hpp       # 全局设备实例声明 (extern)
│   ├── main.cpp                 # 主入口：设备初始化、任务收集、轮询主循环
│   ├── modbus_core.c/h          # RS485 串口通信底层与 CRC-16 校验
├── devices/
│   ├── device_base.hpp          # 设备抽象基类 (DeviceBase / DeviceTask / DeviceStatus)
│   ├── dev_common.c/h           # 公共工具函数
│   ├── dev_cloud_sensors.cpp/hpp# 云测仪 8 路传感器 (PM2.5/PM10/温湿度/TVOC/CH2O/O3/CO2)
│   ├── dev_smoke.cpp/hpp        # 烟雾报警器
│   ├── dev_water.cpp/hpp        # 水浸传感器
│   ├── dev_infrared.cpp/hpp     # 红外+雷达双鉴探测器
│   ├── dev_light.cpp/hpp        # 光感传感器
│   ├── dev_humidifier.cpp/hpp   # 恒湿净化一体机 (读写双向)
│   ├── dev_air_conditioner.cpp/hpp  # 空调集中控制器 (写控制)
│   ├── dev_air_purifier.cpp/hpp # 霉菌空气净化机 (读写双向)
│   └── dev_alarm_device.cpp/hpp # 报警装置/驱鼠器 (线圈控制)
├── service/
│   ├── modbus_service.cpp/hpp   # Modbus 通信服务：串口上下文管理与指令收发
│   └── parse_service.cpp/hpp    # 报文解析服务：CRC 校验 + 数据反序列化
├── ui/                          # Vue 3 前端工程
│   ├── src/
│   │   ├── views/
│   │   │   ├── Login.vue        # 登录页面
│   │   │   └── Dashboard.vue    # 赛博朋克监控大屏 (传感器面板 + ECharts 图表 + 控制面板)
│   │   ├── router/index.ts      # Vue Router 路由配置与登录守卫
│   │   ├── components/          # 公共组件
│   │   ├── App.vue
│   │   └── main.ts
│   ├── package.json
│   ├── vite.config.ts
│   └── dist/                    # 构建产物
├── doc/                         # 设计文档与指令集参考
├── public/                      # 部署用静态资源 (由 Vite 构建产物拷贝)
├── test/                        # 测试文件
├── Makefile                     # C++ 后端构建脚本
└── test.c                       # 测试用例
```

## 编译与运行

### 1. 环境准备 (Ubuntu/Debian)

- `g++` (支持 C++17)
- `make`
- `Node.js` (推荐 v24+) 与 npm
- RS485 硬件串口节点 (默认 `/dev/ttyS9`)，仿真环境可使用本地 pty 接口

### 2. 构建前端

```bash
cd ui
npm install
npm run build
cd ..
# 将构建产物部署到 public 目录供后端托管
cp -r ui/dist/* public/ 2>/dev/null || :
cp ui/dist/index.html dashboard.html
```

### 3. 构建并运行 C++ 后端

```bash
make clean
make

# 默认使用 /dev/ttyS9 串口，在 8080 端口启动 HTTP Server
sudo ./app_gateway
```

自定义串口路径：

```bash
sudo ./app_gateway /dev/ttyUSB0
```

### 4. 访问面板

浏览器打开 `http://localhost:8080`，系统将跳转至登录页面，登录后进入赛博朋克风格的实时监控大屏，支持传感器数据实时刷新与设备远程控制。

## API 接口

### `GET /api/data`

返回所有传感器数据及设备在线状态的 JSON：

```json
{
  “pm25”: 35, “pm25_online”: 1,
  “temperature”: 26, “temperature_online”: 1,
  “humidity”: 55, “humidity_online”: 1,
  “smoke”: 0, “smoke_online”: 1,
  “humidifier_power”: 1, “humidifier_online”: 1,
  “purifier_run_mode”: 2, “purifier_online”: 1
}
```

### `GET /api/control?device=xxx&action=yyy&val=zzz`

下行控制指令，支持的设备与动作：

| device | action | val | 说明 |
|--------|--------|-----|------|
| `ac` | `cool_on` / `cool_off` | - | 空调制冷开/关 |
| `ac` | `heat_on` / `heat_off` | - | 空调制热开/关 |
| `humidifier` | `power` | 0/1 | 恒湿机开关机 |
| `humidifier` | `dehumidify` | 0/1 | 除湿开关 |
| `humidifier` | `humidify` | 0/1 | 加湿开关 |
| `humidifier` | `purify` | 0/1 | 净化开关 |
| `humidifier` | `const_hum` | 0/1 | 恒湿模式开关 |
| `purifier` | `power` | 0/1 | 净化机开关机 |
| `purifier` | `run_mode` | 模式值 | 运行模式设定 |
| `purifier` | `manual` | 0/1 | 手动模式开关 |

## 技术实现

网关运作依赖于 `main.cpp` 中实例化的各个 C++ 设备类，通信流程如下：

1. **初始化**：所有设备实例调用 `init()` 初始化数据缓冲，Web Server 在独立 pthread 中启动。
2. **任务收集**：各设备通过 `getTasks()` 返回 `DeviceTask` 列表（含 readFunc 和 processFunc），统一收集到 `all_tasks` 向量。
3. **轮询主循环**：按序遍历所有任务，调用 `readFunc` 发送 Modbus 请求并接收响应，再由 `processFunc` 完成 CRC 校验、数据解析与实例状态更新。循环间隔 60 秒。
4. **数据上报**：前端通过 `/api/data` 获取全局设备实例的最新数据与在线状态，驱动 ECharts 图表与面板刷新。
5. **远程控制**：前端发起 `/api/control` 请求，后端匹配设备与动作后调用对应设备的 set 方法，动态组帧 Modbus 写指令通过 RS485 下发执行。

## 拓展新设备

1. 在 `devices/` 下新建 `.cpp/.hpp`，继承 `DeviceBase`。
2. 实现 `init()`、`getTasks()`、`getName()` 接口，定义设备的读取与解析逻辑。
3. 在 `core/global_devices.hpp` 中声明全局设备实例 (extern)。
4. 在 `core/main.cpp` 中实例化设备并收集其任务。
5. 在 `application/web_server.cpp` 的 `/api/data` JSON 组装与 `/api/control` 路由中添加对应字段。

## 许可证

本项目仅供学习与研究使用。
