# 电机调速原子服务实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把三台 RS485 电机封装为可编排、可保存、可实时控制和可急停的统一电机调速原子服务。

**Architecture:** `TaskNode` 保存逻辑电机参数，`MotorAtomicService` 管理执行器独占与运行生命周期，`DevStepperMotor` 负责 CiA402 速度协议，所有读写经扩展后的 `CommandQueue` 与传感器共享一个 `AsyncBus`。前端任务编排和运行页只使用逻辑执行器标识，通过 API 获取真实状态。

**Tech Stack:** C++17、Modbus RTU、现有 HTTP/JSON 网关、Vue 3、TypeScript、Vite、项目自带 C++ 测试程序。

**Spec:** `docs/superpowers/specs/2026-09-11-motor-atomic-service-design.md`

## Global Constraints

- `0x02` 使用 IDS57-R 协议并映射为“机舱排烟风机”。
- `0x0E`、`0x0F` 使用 IDS42-R 协议并分别映射为“消防水泵”“舱底排水泵”。
- 用户速度范围为 `1～500 rpm`，默认 `200 rpm`。
- 原子服务不得接收物理地址或任意寄存器值。
- 传感器轮询和电机控制必须共用一个进程内串口调度器。
- 同一逻辑执行器一次只允许一个应用运行实例占用。
- 换向必须先减速至零并确认实际转速，再写入反向速度。
- 停止和急停始终可执行；急停复位前拒绝新的启动。
- 真实模式与模拟模式显式选择，通信失败不得自动切换模拟。

---

### Task 1: 扩展任务参数模型

**Files:**
- Modify: `application/marine/marine_types.hpp`
- Modify: `application/marine/marine_types.cpp`
- Modify: `tests/marine_backend_test.cpp`
- Modify: `ui/src/marine/model.ts`

**Interfaces:**
- Produces: `MotorDirection`, `MotorParameters` 和 `TaskNode::motor`。
- Produces: JSON 字段 `motor.executorId/speedRpm/direction/acceleration/deceleration`。
- Consumes: 现有应用 JSON 保存、加载和参数验证接口。

- [ ] **Step 1: 写参数往返和边界校验失败测试**

在 `tests/marine_backend_test.cpp` 增加测试：构造 `serviceId="M01"` 的节点，设置执行器 `EXHAUST-FAN-01`、速度 `200`、方向 `reverse`、加减速度 `10`，验证 `applicationToJson` 再 `applicationFromJson` 后字段不变；分别将速度设为 `0`、`501`，方向设为 `sideways`，执行器设为 `UNKNOWN`，验证校验失败。

- [ ] **Step 2: 运行测试并确认因 M01 和电机字段尚不存在而失败**

Run: `make test-backend`

Expected: 编译失败或新断言失败，原因明确指向缺失的电机参数模型。

- [ ] **Step 3: 实现 C++ 参数类型、JSON 和验证**

新增：

```cpp
enum class MotorDirection { Forward, Reverse };
struct MotorParameters {
    std::string executorId = "EXHAUST-FAN-01";
    int speedRpm = 200;
    MotorDirection direction = MotorDirection::Forward;
    int acceleration = 10;
    int deceleration = 10;
};
```

把 `M01` 注册为可用执行服务；允许的执行器限定为三个逻辑 ID；速度限定 `1..500`；加减速度限定 `1..100`。序列化使用 `forward/reverse`，旧应用没有 `motor` 字段时只对 `M01` 填默认值。

- [ ] **Step 4: 同步 TypeScript 模型**

在 `TaskNode` 增加可选 `motor`，`createNode('M01')` 生成默认配置，`validateApp` 使用同样的范围和执行器集合，`parseSavedApps` 为旧的 M01 节点补默认值。

- [ ] **Step 5: 运行后端测试和前端类型检查**

Run: `make test-backend`

Run: `cd ui && npm run build`

Expected: 两条命令成功。

- [ ] **Step 6: 提交模型变更**

```bash
git add application/marine/marine_types.hpp application/marine/marine_types.cpp tests/marine_backend_test.cpp ui/src/marine/model.ts
git commit -m "feat: add motor atomic service parameters"
```

### Task 2: 扩展串口队列和电机协议适配器

**Files:**
- Modify: `core/command_queue.hpp`
- Modify: `core/command_queue.cpp`
- Modify: `core/async_bus.hpp`
- Modify: `core/async_bus.cpp`
- Modify: `devices/dev_stepper_motor.hpp`
- Modify: `devices/dev_stepper_motor.cpp`
- Create: `tests/motor_protocol_test.cpp`
- Modify: `Makefile`

**Interfaces:**
- Consumes: `AsyncBus::submit(const AsyncRequest&)`。
- Produces: `CommandQueue::writeRegisters(...)`、`CommandQueue::readRegisters(...)`。
- Produces: `MotorCommandConfig`、`MotorTelemetry` 和 `DevStepperMotor` 的 start/update/stop/read 接口。

- [ ] **Step 1: 写协议编码与顺序失败测试**

创建 `tests/motor_protocol_test.cpp`，通过可注入的命令发送函数记录请求，验证：正向 `200` 编码为 `000000C8`，反向 `200` 编码为 `FFFFFF38`；启动顺序为 `6040=6,7,15`、`6060=3`、`6083`、`6084`、`60FF`；更新速度只写 `60FF`；停止写 `6040=6`；任一步失败后执行停止并返回失败步骤。

- [ ] **Step 2: 运行协议测试并确认失败**

Run: `make test-motor`

Expected: FAIL，因为新接口尚未实现。

- [ ] **Step 3: 为命令队列增加多寄存器写和寄存器读**

扩展 `Command` 以携带最多 24 个寄存器值；构建功能码 `0x10` 和 `0x03` 请求；响应回调验证从机地址、功能码、异常码、寄存器地址或返回字节数。`CommandResult` 保留解析后的寄存器数组和明确错误信息。

- [ ] **Step 4: 重构电机驱动为可测试协议适配器**

`DevStepperMotor` 构造参数包含逻辑 ID、显示名称、型号、地址和 `directionInverted`。实现：

```cpp
MotorCommandResult start(const MotorCommandConfig& config);
MotorCommandResult updateSpeed(int signedRpm);
MotorCommandResult stop(CommandPriority priority = CommandPriority::URGENT);
MotorTelemetry readTelemetry();
```

实际速度从 `606C` 两个寄存器按 I32 解析，状态字从 `6041` 解析；状态仅在有效回帧后更新。

- [ ] **Step 5: 运行协议测试与现有后端测试**

Run: `make test-motor && make test-backend`

Expected: 全部通过。

- [ ] **Step 6: 提交协议层变更**

```bash
git add core/command_queue.hpp core/command_queue.cpp core/async_bus.hpp core/async_bus.cpp devices/dev_stepper_motor.hpp devices/dev_stepper_motor.cpp tests/motor_protocol_test.cpp Makefile
git commit -m "feat: add queued motor speed protocol"
```

### Task 3: 实现电机原子服务、独占资源和安全状态

**Files:**
- Create: `service/atomic/svc_motor_speed.hpp`
- Create: `service/atomic/svc_motor_speed.cpp`
- Modify: `application/marine/marine_executor.hpp`
- Modify: `application/marine/marine_executor.cpp`
- Modify: `application/marine/marine_runtime.hpp`
- Modify: `application/marine/marine_runtime.cpp`
- Modify: `application/marine/marine_safety.hpp`
- Modify: `application/marine/marine_safety.cpp`
- Modify: `tests/marine_backend_test.cpp`
- Modify: `Makefile`

**Interfaces:**
- Consumes: Task 1 的 `MotorParameters` 和 Task 2 的 `DevStepperMotor`。
- Produces: `MotorAtomicService::start/update/stop/telemetry/emergencyStop/resetEmergency`。
- Produces: 运行时节点真实 `ServiceResult` 和资源冲突错误。

- [ ] **Step 1: 写资源锁、生命周期和急停失败测试**

用内存电机适配器验证：第一个 run 获取 `EXHAUST-FAN-01` 后第二个 run 被拒绝；节点完成、取消和失败均调用 stop 并释放；急停停止全部已占用电机并阻止新启动；复位后允许启动；模拟模式只改变内存遥测，不调用真实发送器。

- [ ] **Step 2: 运行测试并确认新行为失败**

Run: `make test-backend`

Expected: FAIL，原因是 `MotorAtomicService` 与运行时钩子缺失。

- [ ] **Step 3: 实现 MotorAtomicService**

服务内部维护三个逻辑执行器、`executorId -> runId/nodeId` 独占表、急停锁存状态和真实/模拟模式。所有修改在互斥锁下完成，串口等待在锁外完成，完成后以所有权令牌校验结果，避免旧回调覆盖新状态。

- [ ] **Step 4: 接入 MarineRuntime 生命周期**

在 M01 节点进入 Running 时启动电机；进度期间刷新遥测；节点完成、run 取消、run 失败、runtime stop 时调用 stop。暂停应用时受控停止，恢复时重新获取资源并按保存参数启动。通信失败把节点与 run 标记 Failed，并写事件。

- [ ] **Step 5: 修改安全模型**

删除 `false &&` 硬锁。真实控制默认可用；新增 `emergencyStopped` 锁存状态。`mayStart()` 在急停时拒绝，`mayStop()` 始终返回允许。保留审计日志。

- [ ] **Step 6: 运行全部 C++ 测试**

Run: `make test-motor && make test-backend`

Expected: 全部通过。

- [ ] **Step 7: 提交服务与运行时变更**

```bash
git add service/atomic/svc_motor_speed.hpp service/atomic/svc_motor_speed.cpp application/marine/marine_executor.hpp application/marine/marine_executor.cpp application/marine/marine_runtime.hpp application/marine/marine_runtime.cpp application/marine/marine_safety.hpp application/marine/marine_safety.cpp tests/marine_backend_test.cpp Makefile
git commit -m "feat: execute motor atomic services"
```

### Task 4: 接入网关设备与 HTTP API

**Files:**
- Modify: `core/main.cpp`
- Modify: `core/global_devices.hpp`
- Modify: `core/device_config.h`
- Modify: `application/marine/marine_api.hpp`
- Modify: `application/marine/marine_api.cpp`
- Modify: `application/marine/marine_repository.cpp`
- Modify: `tests/marine_backend_test.cpp`

**Interfaces:**
- Consumes: Task 3 的 `MotorAtomicService`。
- Produces: `GET /api/v1/marine/motors`。
- Produces: `PATCH /api/v1/marine/runs/{runId}/nodes/{nodeId}/motor`。
- Produces: `POST /api/v1/marine/motors/{executorId}/stop`。
- Produces: `POST /api/v1/marine/emergency-stop` 和 `/emergency-reset`。

- [ ] **Step 1: 写 API 失败测试**

验证电机列表包含三个逻辑执行器和型号；动态速度 `500` 成功、`501` 返回 400；非所有者更新返回 409；急停返回所有电机停止结果并使 health 显示锁存；复位恢复控制；停止接口在急停状态仍可用。

- [ ] **Step 2: 运行后端测试并确认端点不存在**

Run: `make test-backend`

Expected: 新端点测试返回 404。

- [ ] **Step 3: 注册三台真实设备**

将全局设备定义改为 `0x02/0x0E/0x0F`，分别配置 IDS57-R、IDS42-R、IDS42-R 和确定的显示名称。在串口、异步总线和命令队列启动后构造并注入 `MotorAtomicService`，保证 marine runtime 不持有尚未初始化的串口依赖。

- [ ] **Step 4: 实现 API 与错误映射**

解析路径和 JSON；参数错误返回 400，资源冲突返回 409，急停锁定返回 423，设备错误返回 502，通信超时返回 504。所有响应携带 `mode`、`source`、`updatedAt`，写操作附加审计事件。

- [ ] **Step 5: 增加启动模式**

保留现有 `--marine-demo` 为 simulation；普通串口启动为 real。health 返回 `motorMode`、`actuatorControlEnabled` 和 `emergencyStopped`，不得基于串口错误切换模式。

- [ ] **Step 6: 运行测试并构建网关**

Run: `make test-motor && make test-backend && make app_gateway`

Expected: 全部成功，编译无新增 warning。

- [ ] **Step 7: 提交网关 API 变更**

```bash
git add core/main.cpp core/global_devices.hpp core/device_config.h application/marine/marine_api.hpp application/marine/marine_api.cpp application/marine/marine_repository.cpp tests/marine_backend_test.cpp
git commit -m "feat: expose motor control through gateway"
```

### Task 5: 完成前端编排、运行控制和数据流展示

**Files:**
- Modify: `ui/src/marine/api.ts`
- Modify: `ui/src/marine/model.ts`
- Modify: `ui/src/views/MarineDemo.vue`
- Modify: `ui/src/components/marine/ApplicationCommandCenter.vue`
- Modify: `ui/src/components/marine/ApplicationFlow.vue`
- Modify: `ui/src/styles/marine.css`
- Modify: `ui/tests/marine.test.mjs`

**Interfaces:**
- Consumes: Task 4 的 motor API。
- Produces: 编排参数编辑、运行时电机控制、急停和实时电机数据流。

- [ ] **Step 1: 写前端模型与 API 失败测试**

测试 M01 默认参数、边界校验、旧记录迁移、PATCH 请求体和真实/模拟模式标识。测试名称直接描述用户行为，例如“反转配置保存后仍为反转”。

- [ ] **Step 2: 运行测试并确认失败**

Run: `cd ui && npm run test:marine`

Expected: 新测试因 M01 UI/API 缺失而失败。

- [ ] **Step 3: 实现任务编排参数面板**

服务库加入“电机调速控制”。选中节点时展示三台逻辑执行器下拉框、1～500 rpm 输入与滑杆、正反转、持续时间；高级区展示 1～100 的加减速度。节点卡片直接显示电机名、方向和 rpm。

- [ ] **Step 4: 实现应用运行控制**

运行卡片每秒刷新电机遥测，显示目标/实际 rpm、方向、状态字、在线、型号、绑定设备和占用应用。提供动态调速、受控换向、停止按钮；请求期间禁用重复操作并显示当前阶段。

- [ ] **Step 5: 实现固定急停和数据流**

运行页固定显示红色“执行器急停”，二次确认后调用 API；急停状态显示复位操作。`ApplicationFlow` 增加“应用 → 电机调速原子服务 → 逻辑执行器 → IDS42/57 适配器 → 物理电机”的粒子流，按命令、回执、实际转速三种状态改变颜色和动画速度。

- [ ] **Step 6: 运行前端测试与构建**

Run: `cd ui && npm run test:marine`

Run: `cd ui && npm run build`

Expected: 测试和构建成功。

- [ ] **Step 7: 提交前端功能**

```bash
git add ui/src/marine/api.ts ui/src/marine/model.ts ui/src/views/MarineDemo.vue ui/src/components/marine/ApplicationCommandCenter.vue ui/src/components/marine/ApplicationFlow.vue ui/src/styles/marine.css ui/tests/marine.test.mjs
git commit -m "feat: configure and operate motor services"
```

### Task 6: 端到端验证与部署文档

**Files:**
- Modify: `README.md`
- Modify: `docs/DEVELOPMENT_STATUS.md`
- Create: `docs/MOTOR_ATOMIC_SERVICE.md`
- Modify: `docs/OFFLINE_DEPLOYMENT.md`

**Interfaces:**
- Consumes: 完整后端和前端。
- Produces: 可重复的模拟验收、实机验收和离线部署说明。

- [ ] **Step 1: 执行完整自动验证**

Run: `make test-motor && make test-backend && make frontend && make app_gateway`

Expected: 全部成功。

- [ ] **Step 2: 运行模拟模式端到端流程**

以 `./app_gateway --marine-demo` 启动，创建包含 M01 的应用，验证保存、运行、调速、换向、资源冲突、取消、急停和复位。记录调用接口、预期响应和页面操作。

- [ ] **Step 3: 浏览器检查会议大屏页面**

使用 `127.0.0.1` 打开前端，检查 1920×1080 下无小字、无溢出、参数控件完整、真实/模拟标识清楚、数据流动画连贯。

- [ ] **Step 4: 编写运行与实机验收文档**

说明两种型号和地址、启动命令、模式选择、参数含义、急停、恢复、日志位置及实机测试顺序。明确每次实机测试从 200 rpm 开始，现场确认后再提高，最大 500 rpm。

- [ ] **Step 5: 执行三台实机验收**

停止其他串口进程，通过正式 API 对三台电机逐台执行 200 rpm 正转、300 rpm 调速、受控反转、停止；再执行三台并行和全局急停。核对 `606C` 实际速度与现场动作。若方向与船舶语义相反，只修改该设备的 `directionInverted`。

- [ ] **Step 6: 更新文档并提交**

```bash
git add README.md docs/DEVELOPMENT_STATUS.md docs/MOTOR_ATOMIC_SERVICE.md docs/OFFLINE_DEPLOYMENT.md
git commit -m "docs: document motor service operation"
```

- [ ] **Step 7: 最终状态检查**

Run: `git status --short`

Expected: 只显示开始实施前已经存在且未纳入本功能的工作区改动；本计划涉及的文件全部已提交。
