# 八防综合监控网关平台 (Multi-Defense Sensor Gateway)

基于 C 语言开发的一款轻量级、模块化、事件驱动的 Modbus RTU 传感器网关。该项目通过 RS485 串口定时轮询各类环境与安防传感器，对报文进行校验和业务解析，同时在后台独立线程上运行一个轻量的 C Web Server，为前端实时大屏端点 (`dashboard.html`) 提供 JSON 数据。

## 🌟 特性

- **基于表驱动法 (Table-Driven)**：无需处理庞大臃肿的 `switch-case`，一行注册表配置即可轻松扩展数百个传感器轮询任务。
- **全自动 CRC-16 校验构建**：发送指令动态拼接并自动计算 Modbus CRC-16 与返回数据的校验，无需手工计算任何十六进制常数。
- **多线程并行架构**：独立的前端通信线程（`start_web_server`）与后端的 Modbus 轮询采集线程分离，通过 `pthread_mutex_t` 互斥锁确保共享状态（`g_sensor_data`）访问安全。
- **多设备分离解析 (Modular Design)**：彻底解耦协议层与设备业务层，每个独立的硬件设备都在独立的 `.c` 模块实现专属清洗过滤规则。

## 📂 项目结构

```text
├── core/                        # 核心组件库
│   ├── device_config.h          # 传感器的 Modbus 寄存器及地址常量统一定义
│   ├── modbus_core.c/h          # 底层串口通信 (RS485)、报文封包发包及 CRC 校验
│   ├── sensor_data.c/h          # 多线程安全的全局传感器内存缓存 (SensorData)
│   └── web_server.c/h           # 轻量级 HTTP Socket Server (提供 /api/data)
├── devices/                     # 各种硬件设备的业务解析层代码
│   ├── dev_cloud.c/h            # 空气质量/环境传感器读写和解析
│   ├── dev_infrared.c/h         # 红外雷达双鉴传感器读写和解析
│   ├── dev_light.c/h            # 环境光照度传感器读写和解析
│   ├── dev_smoke.c/h            # 烟雾/火警侦测传感器读写和解析
│   └── dev_water.c/h            # 水浸漏水检测传感器读写和解析
├── dashboard.html               # 纯前端数据可视化监控大屏面板
├── main.c                       # 主程序：包含任务执行表以及多线程服务拉起
└── Makefile                     # 编译构建脚本 (-lpthread)
```

## 🚀 编译与运行

**1. 准备环境**
系统需安装 GCC 构建工具并支持 POSIX 进程线程 API (`pthread`)，硬件需要有可写权限的 RS485 串口节点，目前系统默认为 `/dev/ttyS9`。

**2. 编译项目**
在根目录执行以下命令：
```bash
make clean
make
```
即可在根目录生成编译好的可执行文件 `app_gateway`。

**3. 运行程序**
```bash
# 默认监听 /dev/ttyS9 串口，并同时在 8080 端口启动 Web API。
sudo ./app_gateway
```

也可以通过传递参数自定义串口路径（如 `/dev/ttyUSB0`）：
```bash
sudo ./app_gateway /dev/ttyUSB0
```

**4. 访问大屏**
通过浏览器直接访问 `http://localhost:8080`，你可以看到每数秒局部刷新一次数据的实时动态可视化控制面板。

## ⚙ 技术实现与代码逻辑

网关运作依赖于 `main.c` 中注册的**轮询任务一览表** `device_task_t` 型数组。
每当网关启动，会先调用并衍生一条 `start_web_server` 线程绑定 8080 端口进行阻塞监听。
主线程进入死循环后持续遍历上述的轮询表：
1. 提取任务表定义的设备请求函数（如 `cloud_read_pm25`）。
2. 构建标准 RS485 数据帧经过内核 Socket 发送到物理串口。
3. 等待回应后由 `modbus_core.c` 接管执行基于长度和字节的基础截断。
4. 交付响应给任务表定义的相应解析器（如 `cloud_process_pm25`）。
5. 专用解析器在剥离业务异常包或 CRC 错误后，尝试争夺 `g_sensor_data.lock` 全局互斥锁，成功后将有效转化后的整型/浮点型数值推入全量状态机。
6. 并发的 HTTP 服务端在接获 `/api/data` 的 GET 请求后即时释放上述结构体为一串标准 JSON 交由 `dashboard.html` 的定时器消费刷新。

## 🔧 如何添加新设备 API 或拓展功能？

如果在后续工作中引进了一个**新的传感器**（例如甲烷报警器），基于目前的表驱动设计，开发者**无需打乱主线循环**。请遵循如下步骤进行对接：

**Step 1: 全局地址分配**
在 `core/device_config.h` 中分配并录入新设备的从站地址或寄存器起步段，例如：
```c
#define DEV_METHANE_ADDR  0x05
#define REG_METHANE_STATE 0x0001
```

**Step 2: 建立对应驱动文件**
在 `devices/` 目录下创建一个 `dev_methane.h` 和 `dev_methane.c`，编写该设备的**下发包裹请求宏**与**接收解码器**：
```c
#include "dev_methane.h"
#include "sensor_data.h"
#include "modbus_core.h"
#include "device_config.h"

// 请求发送
int methane_read_state(const char* device, uint8_t *resp, size_t *resp_len) {
    return modbus_build_and_send(device, 9600, DEV_METHANE_ADDR, 0x03, REG_METHANE_STATE, 0x0001, resp, 512, resp_len, 300);
}

// 接收解析
void methane_process_data(const uint8_t *resp, size_t resp_len) {
    // 处理报文有效性并获取甲烷数值 (略)
    uint16_t value = ...;

    // 争夺锁存入共享中心数据
    pthread_mutex_lock(&g_sensor_data.lock);
    g_sensor_data.methane_ppm = value; 
    pthread_mutex_unlock(&g_sensor_data.lock);
}
```

**Step 3: 扩充全局存储与 API 分发**
修改 `core/sensor_data.h`，在 `SensorData` 的结构体中分配出 `methane_ppm` 以支持内存缓存。
在 `core/web_server.c` 中修改 JSON 渲染语句把 `g_sensor_data.methane_ppm` 向暴露的 API 追加抛出。

**Step 4: 注册入主引擎轮询链 (`main.c`)**
将写好的驱动头文件引入到 `main.c` 的顶部。
找到 `device_task_t tasks[]`，加入一行你崭新的指令描述即可！
```c
// ... 其他已存在设备轮询表
{"Methane Detector - PPM", methane_read_state, methane_process_data},
```

**Step 5: 添加进编译脚本**
切记去编辑根目录的 `Makefile` 并将其录入 `SRCS` 行中 `devices/dev_methane.c` 即可执行 `make` 得到最新的工程。如果你还需要展示在网页上，接着只需同步在前端 `dashboard.html` 内建一个数据卡片绑定其更新的 `id` 即可双端同步上线！
