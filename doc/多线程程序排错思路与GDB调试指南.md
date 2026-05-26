# 多线程程序排错思路与 GDB 调试指南

> 基于本次 IoT 网关项目的实际排错经验整理 | 2026-05-26

---

## 目录

- [1. 问题现象与初步判断](#1-问题现象与初步判断)
- [2. 排错思路：从现象到根因](#2-排错思路从现象到根因)
- [3. 代码级根因分析](#3-代码级根因分析)
  - [3.1 Bug 1：EnvDataCpp 非递归 mutex 死锁](#31-bug-1envdatacpp-非递归-mutex-死锁)
  - [3.2 Bug 2：transact() 持锁阻塞导致 close() 30秒卡死](#32-bug-2transact-持锁阻塞导致-close-30秒卡死)
  - [3.3 Bug 3：多线程信号投递与 select() 中断](#33-bug-3多线程信号投递与-select-中断)
  - [3.4 Bug 4：select() 无法被 close(fd) 唤醒](#34-bug-4select-无法被-closefd-唤醒)
- [4. GDB 调试多线程程序](#4-gdb-调试多线程程序)
- [5. 常见多线程问题的 GDB 排查流程](#5-常见多线程问题的-gdb-排查流程)
- [6. 本次项目排错复盘](#6-本次项目排错复盘)
- [7. 面试高频问题与回答要点](#7-面试高频问题与回答要点)

---

## 1. 问题现象与初步判断

### 1.1 现象一：Ctrl+C 无法终止程序

**现象描述：**
- 程序启动后正常运行，按 Ctrl+C 后程序无反应
- 需要 `kill -9` 才能强制终止

**初步判断思路：**

| 可能原因 | 判断方法 | 优先级 |
|---------|---------|-------|
| 信号未注册 | 检查代码中是否有 `signal()` 或 `sigaction()` | 高 |
| 信号被阻塞 | `cat /proc/<pid>/status \| grep SigBlk` | 中 |
| 信号处理函数未生效 | 在信号处理函数中加 `write(STDERR_FILENO, ...)` 确认是否被调用 | 高 |
| 主循环未检查退出标志 | 检查循环条件和退出标志变量 | 高 |
| `sleep()`/`select()` 未被中断 | 检查是否设置了 `SA_RESTART` | 高 |

**关键知识点：**
- `SA_RESTART` 标志会让被信号中断的系统调用自动重启，导致 `read()`/`select()`/`nanosleep()` 不返回
- 多线程程序中，信号会被投递到任意未阻塞的线程，不一定是主线程
- `usleep()` 在某些实现中不会被信号中断

### 1.2 现象二：程序退出极慢（30+ 秒）

**现象描述：**
- Ctrl+C 后程序最终能退出，但需要 30 秒以上
- 日志显示某个清理步骤耗时异常

**初步判断思路：**

| 可能原因 | 判断方法 | 优先级 |
|---------|---------|-------|
| 死锁 | `gdb attach <pid>` 查看线程堆栈 | **最高** |
| 阻塞 I/O 未超时 | 检查 `read()`/`select()` 的超时参数 | 高 |
| 线程 join 超时 | 检查 `pthread_join()` 是否有超时机制 | 中 |
| 资源未释放 | 检查文件描述符、锁的释放顺序 | 高 |

**快速定位方法：**
```bash
# 查看进程在做什么
strace -p <pid> -e trace=read,write,select,futex

# 查看线程状态
cat /proc/<pid>/task/*/status | grep -E 'Name|State'

# 查看锁的持有者
cat /proc/<pid>/task/*/wchan
```

### 1.3 现象三：网页无法访问

**现象描述：**
- 程序启动后，浏览器访问 `http://localhost:8080` 无响应
- curl 也返回超时

**初步判断思路：**

| 可能原因 | 判断方法 | 优先级 |
|---------|---------|-------|
| 程序启动阶段死锁 | 检查日志，看程序是否到达 Web 服务器启动步骤 | **最高** |
| 端口被占用 | `ss -tlnp \| grep 8080` | 高 |
| Web 线程未创建 | `ps -eLf \| grep <pid>` 查看线程数 | 中 |
| 线程初始化死锁 | 某个设备初始化函数死锁导致后续代码不执行 | 高 |

---

## 2. 排错思路：从现象到根因

### 2.1 排错五步法

```
现象 → 时间线 → 缩小范围 → 定位根因 → 验证修复
```

**Step 1：记录现象**
- 什么时候发生的？（启动时 / 运行中 / 退出时）
- 必现还是偶发？
- 有无日志输出？日志停在哪一步？

**Step 2：建立时间线**
- 从日志中提取关键时间戳
- 计算每步之间的耗时
- 找到耗时异常的步骤

**Step 3：缩小范围**
- 是信号处理问题？→ 检查信号注册和处理函数
- 是死锁问题？→ 检查锁的获取顺序
- 是阻塞 I/O 问题？→ 检查超时参数

**Step 4：定位根因**
- 使用 GDB 附加到运行中的进程
- 查看各线程的调用栈
- 检查锁的状态

**Step 5：验证修复**
- 修复后重新编译运行
- 确认问题不再复现
- 检查是否引入新问题

### 2.2 本次项目的排查时间线

```
现象: Ctrl+C 无反应 + 网页打不开
  ↓
Step 1: 检查日志 → 日志显示程序启动正常，但初始化阶段有死锁
  ↓
Step 2: 定位死锁 → EnvDataCpp 的 init() 函数中 mutex 二次加锁
  ↓
Step 3: 修复死锁 → 移除外层 lock_guard
  ↓
Step 4: 重新测试 → 网页可访问，但 Ctrl+C 仍需 30+ 秒
  ↓
Step 5: 分析退出流程 → serialBus.close() 等待轮询线程释放 mutex
  ↓
Step 6: 定位根因 → transact() 在 I/O 期间持有 mutex
  ↓
Step 7: 修复 → 添加 shutdown pipe + 释放 I/O 锁
  ↓
Step 8: 验证 → Ctrl+C 1 秒内退出
```

---

## 3. 代码级根因分析

本节从代码层面详细分析每个 Bug 的成因、触发条件和修复方案。

### 3.1 Bug 1：EnvDataCpp 非递归 mutex 死锁

#### Bug 1 问题代码

```cpp
// devices/device_base.hpp — EnvDataCpp 结构体
struct EnvDataCpp {
    mutable std::mutex mtx;          // ← 非递归 mutex
    int co2 = 0, ch2o = 0, tvoc = 0, pm25 = 0, pm10 = 0;

    // 赋值运算符: 内部加锁
    EnvDataCpp& operator=(const EnvDataCpp& other) {
        if (this != &other) {
            std::scoped_lock lock(mtx, other.mtx);  // ← 第二次加锁!
            co2 = other.co2; ch2o = other.ch2o;
            tvoc = other.tvoc; pm25 = other.pm25; pm10 = other.pm10;
        }
        return *this;
    }
};

// devices/dev_humidifier.cpp — init() 函数
void DevHumidifier::init() {
    status_.reset();
    {
        std::lock_guard<std::mutex> lock(env_.mtx);  // ← 第一次加锁 (持有 env_.mtx)
        env_ = EnvDataCpp{};  // ← 调用 operator=, 内部再次 scoped_lock(env_.mtx, ...)
    }                         //    std::mutex 不可重入 → 死锁!
    power_state_.store(0);
    fault_state_.store(0);
}
```

#### Bug 1 死锁触发链

```
主线程调用 init_all_devices()
  → DevHumidifier::init()
    → lock_guard<std::mutex>(env_.mtx)     // 加锁成功, env_.__lock = 1
    → env_ = EnvDataCpp{}
      → EnvDataCpp::operator=()
        → scoped_lock(env_.mtx, other.mtx) // 尝试再次加锁 env_.mtx
        → __pthread_mutex_lock_full()      // env_.__lock 已经是 1
        → futex_wait()                     // 同一线程等待自己释放锁 → 死锁!
```

#### Bug 1 为什么是死锁而不是 UB？

`std::mutex` 对同一线程二次加锁是 **undefined behavior**。在 glibc 的实现中，`pthread_mutex_lock` 默认使用 `PTHREAD_MUTEX_NORMAL` 类型，同一线程二次加锁会永久阻塞（不是返回错误 `EDEADLK`）。`EDEADLK` 只在 `PTHREAD_MUTEX_ERRORCHECK` 类型下才会返回。

#### Bug 1 为什么 init() 中会有外层 lock_guard？

原始设计意图是：在赋值前先锁住 `env_.mtx`，确保赋值操作的原子性。但开发者没有意识到 `operator=` 内部也会加锁。这是一个典型的 **"不知道被调用函数内部已有锁保护"** 的问题。

#### Bug 1 修复方案

```cpp
// 修复后: 移除外层 lock_guard
void DevHumidifier::init() {
    status_.reset();
    env_ = EnvDataCpp{};   // operator= 内部的 scoped_lock 足够保护
    power_state_.store(0);
    fault_state_.store(0);
}
```

移除外层 `lock_guard` 后，`operator=` 内部的 `scoped_lock` 已经提供了完整的互斥保护。赋值操作本身就是原子的（单个 `operator=` 调用），不需要外层额外加锁。

#### Bug 1 触发条件

- `init()` 在 `main()` 中被调用（单线程环境）
- `DevHumidifier::init()` 和 `DevAirPurifier::init()` 都有此问题
- 一旦调用 `init()`，程序立即死锁，后续所有代码（包括 Web 服务器）都不会执行

---

### 3.2 Bug 2：transact() 持锁阻塞导致 close() 30秒卡死

#### Bug 2 问题代码

```cpp
// core/serial_bus.cpp — 优化前的 transact()
int SerialBus::transact(const uint8_t *request, size_t request_len,
                        uint8_t *response, size_t response_cap,
                        size_t *response_len, int timeout_ms) {
    // 整个函数体都在锁的保护下
    std::unique_lock<std::recursive_mutex> lock(mtx_);  // ← 加锁

    stats_.totalTransactions++;

    if (fd_ < 0) { /* 重连逻辑 */ }

    usleep(inter_frame_delay_us_);     // ← 帧间延时 (~5ms), 持锁等待
    flushBuffers();                     // ← 持锁
    rawSend(request, request_len);      // ← 持锁, 串口写入
    rawRecv(response, ..., timeout_ms); // ← 持锁, 阻塞在 select() 上!
    //   ↑ timeout_ms 通常为 1000ms (1秒)
    //   ↑ 在这 1 秒内, 其他线程无法获取 mutex

    // ... 统计更新 ...
    return 0;
}  // 释放锁

// core/serial_bus.cpp — close()
void SerialBus::close() {
    std::lock_guard<std::recursive_mutex> lock(mtx_);  // ← 尝试获取锁
    // 如果 20 个轮询线程都在 transact() 中阻塞, 这里要等 20 × 1秒 = 20秒
    // 加上帧间延时和争用, 实际等待 ~30 秒
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}
```

#### Bug 2 阻塞链分析

```
时间线 (20 个轮询线程, 每个 transact 持锁 ~1.5秒):

Thread 1:  |---- transact (持锁 1.5s) ----|---- transact (持锁 1.5s) ----| ...
Thread 2:    |---- transact (持锁 1.5s) ----|---- transact (持锁 1.5s) --| ...
Thread 3:      |---- transact (持锁 1.5s) ----|---- transact (持锁 1.5s) | ...
...
Thread 20:                                                       |---- transact ...

主线程:                                                              |close() 等锁...|
                                                                   ↑ SIGINT 到达
                                                                   等待 Thread 1~20 依次释放 mutex
                                                                   总计 ~30 秒
```

#### Bug 2 为什么是 30 秒？

- 20 个轮询线程，每个 transact 持锁约 1.5 秒（帧间延时 5ms + 发送 + select 超时 1000ms + 接收 + 统计）
- `close()` 需要等待所有 20 个线程的当前 transact 完成
- 20 × 1.5s = 30s

#### Bug 2 日志证据

```
08:40:46.272 [INFO] main.cpp:402 - 正在关闭串口...     ← close() 开始等待
08:41:15.793 [INFO] main.cpp:405 - 正在停止轮询...     ← close() 返回, 等了 29.5 秒
```

#### Bug 2 修复方案

**核心思想：** 将 mutex 的保护范围缩小，不在 I/O 期间持锁。

```cpp
// 修复后的 transact()
int SerialBus::transact(...) {
    int local_fd;

    // 第一阶段: 短暂持锁, 检查 fd 和重连
    {
        std::unique_lock<std::recursive_mutex> lock(mtx_);
        stats_.totalTransactions++;
        if (fd_ < 0) { /* 重连 */ }
        local_fd = fd_;
    }  // ← 释放锁! I/O 不持锁

    // 第二阶段: I/O 操作, 无锁
    usleep(inter_frame_delay_us_);
    tcflush(local_fd, TCIOFLUSH);
    rawSend(request, request_len);      // 使用 local_fd, 不访问 fd_
    rawRecv(response, ..., timeout_ms); // 使用 local_fd, 不访问 fd_

    // 第三阶段: 更新统计, 短暂持锁
    if (rc != 0) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        stats_.totalErrors++;
    }
    return 0;
}
```

**关键改动：**

1. `rawSend()` 和 `rawRecv()` 内部将 `fd_` 捕获为局部变量 `int fd = fd_`，避免在无锁状态下访问可能被修改的 `fd_`
2. `close()` 只需短暂持锁（关闭 fd + 设置 fd_ = -1），不会被 I/O 阻塞
3. 轮询线程的 I/O 失败后会检查 `running_` 标志并退出

---

### 3.3 Bug 3：多线程信号投递与 select() 中断

#### Bug 3 问题代码

```cpp
// core/main.cpp — 原始信号处理
// 方案 1: 使用 signal() + usleep()
signal(SIGINT, [](int) { g_running = false; });
while (g_running) {
    usleep(1000000);  // 1秒
    // 打印统计...
}
```

#### Bug 3 为什么 usleep() 不被中断？

1. **信号投递到错误的线程：** 多线程程序中，SIGINT 会被投递到任意未阻塞信号的线程。如果 SIGINT 被轮询线程接收，主线程的 `usleep()` 不会被中断。

2. **SA_RESTART 标志：** 如果使用 `signal()` 而非 `sigaction()`，某些系统会默认设置 `SA_RESTART`，导致被中断的系统调用自动重启。

3. **usleep 的实现：** 某些 libc 实现中，`usleep()` 内部使用 `nanosleep()`，而 `nanosleep()` 会被信号中断并返回剩余时间。但如果信号被其他线程接收，主线程的 `nanosleep()` 根本不会被中断。

#### Bug 3 修复方案：Self-Pipe Trick

```cpp
// core/main.cpp — 修复后的信号处理
static int g_signal_pipe[2] = {-1, -1};

static void signal_handler(int sig) {
    (void)sig;
    g_web_running = false;
    // 写一个字节到管道, 唤醒 select()
    char c = 'X';
    write(g_signal_pipe[1], &c, 1);
}

int main() {
    // 创建管道
    pipe(g_signal_pipe);
    fcntl(g_signal_pipe[0], F_SETFL, O_NONBLOCK);

    // 注册信号 (不设置 SA_RESTART)
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // ← 关键: 不设置 SA_RESTART
    sigaction(SIGINT, &sa, NULL);

    // 主循环: select() 监听管道
    while (g_web_running) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_signal_pipe[0], &rfds);
        struct timeval tv = {1, 0};  // 1秒超时

        int sel_rc = select(g_signal_pipe[0] + 1, &rfds, NULL, NULL, &tv);
        // select 返回的三种情况:
        // 1. sel_rc > 0: 管道可读 → 信号到达 → g_web_running 已为 false
        // 2. sel_rc == 0: 超时 → 打印统计 → 继续循环
        // 3. sel_rc < 0, errno == EINTR: 被信号中断 → 检查 g_web_running

        if (FD_ISSET(g_signal_pipe[0], &rfds)) {
            char buf[16];
            while (read(g_signal_pipe[0], buf, sizeof(buf)) > 0) {} // drain
        }
        if (!g_web_running) break;
        // 打印统计...
    }
}
```

#### Bug 3 Self-Pipe Trick 的原理

```
信号到达 (SIGINT)
  → signal_handler() 被调用 (在任意线程中)
  → g_web_running = false
  → write(g_signal_pipe[1], 'X')   // 写管道

主线程:
  → select() 监听 g_signal_pipe[0]
  → 管道变为可读 → select() 立即返回
  → 检测到 g_web_running == false → 退出循环
```

**为什么可靠？**

- 无论信号被哪个线程接收，`write()` 都会写入管道
- 管道是全局的，`select()` 始终监听它
- `select()` 会被管道可读事件唤醒，不依赖信号中断

---

### 3.4 Bug 4：select() 无法被 close(fd) 唤醒

#### Bug 4 问题描述

即使修复了 Bug 2（I/O 不持锁），`serialBus.close()` 关闭串口 fd 后，阻塞在 `select(fd, ...)` 上的轮询线程不会立即返回。

#### Bug 4 原因分析

Linux 内核中，`select()` 监控的是 **文件描述符表中的条目**，而非 fd 编号本身。当一个线程调用 `close(fd)` 时：

1. fd 编号被从文件描述符表中移除
2. 但另一个线程的 `select()` 已经在内核中注册了对这个 fd 的等待
3. 内核不会主动唤醒这个 `select()` 调用
4. `select()` 只能等到超时才返回

这是 POSIX 标准的灰色地带——不同操作系统的行为不同：

- **Linux：** `select()` 不会被 `close()` 唤醒，等到超时返回
- **macOS：** `select()` 会返回 `EBADF`
- **Solaris：** 行为不确定

#### Bug 4 修复方案：Shutdown Pipe

```cpp
// core/serial_bus.hpp
class SerialBus {
    int shutdown_pipe_[2] = {-1, -1};  // 用于中断 select() 的管道
    // ...
};

// 构造函数: 创建管道
SerialBus::SerialBus(const char *device, int baud)
    : device_(device), baud_(baud) {
    pipe(shutdown_pipe_);
    fcntl(shutdown_pipe_[0], F_SETFL, O_NONBLOCK);
    fcntl(shutdown_pipe_[1], F_SETFL, O_NONBLOCK);
}

// rawRecv(): select() 同时监听串口 fd 和 shutdown pipe
int SerialBus::rawRecv(uint8_t *buf, size_t buf_cap, size_t *recv_len, int timeout_ms) {
    int fd = fd_;
    int pipe_fd = shutdown_pipe_[0];
    int nfds = (fd > pipe_fd ? fd : pipe_fd) + 1;

    while (total_read < buf_cap) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        if (pipe_fd >= 0) FD_SET(pipe_fd, &readfds);  // ← 同时监听管道

        int sel = select(nfds, &readfds, NULL, NULL, &tv);
        if (sel < 0) { if (errno == EINTR) continue; return -1; }
        if (sel == 0) break;

        // shutdown pipe 可读 → 正在关闭
        if (pipe_fd >= 0 && FD_ISSET(pipe_fd, &readfds)) {
            return -1;  // ← 立即返回, 不等超时
        }

        ssize_t n = read(fd, buf + total_read, buf_cap - total_read);
        // ...
    }
    return 0;
}

// close(): 写管道唤醒所有阻塞在 select() 上的线程
void SerialBus::close() {
    // 先写管道唤醒阻塞的 select()
    if (shutdown_pipe_[1] >= 0) {
        char c = 'X';
        write(shutdown_pipe_[1], &c, 1);  // ← 唤醒所有 select()
    }
    // 再关闭串口 fd
    std::lock_guard<std::recursive_mutex> lock(mtx_);
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}
```

#### Bug 4 唤醒流程

```
SIGINT 到达
  → signal_handler: g_web_running = false
  → 主循环退出
  → serialBus.close()
    → write(shutdown_pipe_[1], 'X')   // 写管道
    → 所有阻塞在 select() 的线程被唤醒
    → 每个线程检测到 pipe_fd 可读 → return -1
    → transact() 返回错误
    → pollGroupOnce() 记录失败
    → pollGroupThread() 检查 running_ == false → 退出循环
  → poller.stop()
    → 所有线程已退出 → join() 立即返回
  → 清理完成, 进程退出
```

#### Bug 4 为什么不用 pthread_kill？

另一种方案是用 `pthread_kill(thread, SIGUSR1)` 中断 `select()`。但缺点是：
1. 需要记录所有轮询线程的 `pthread_t`
2. `SIGUSR1` 的处理函数需要正确设置（不能 terminate 进程）
3. 如果线程不在 `select()` 中，信号会延迟处理

Shutdown pipe 方案更简洁可靠，是 Unix 网络编程中的标准模式（又称 "self-pipe trick" 的变体）。

---

## 4. GDB 调试多线程程序

### 4.1 基本准备

```bash
# 编译时加入调试信息
g++ -g -O0 -pthread -o app_gateway ...

# 或者在 Makefile 中修改
CFLAGS = -Wall -std=c++17 -g -O0
```

**重要：** `-O0` 禁用优化，确保变量值和调用栈准确。

### 4.2 启动 GDB 的三种方式

```bash
# 方式 1: 直接启动
gdb ./app_gateway

# 方式 2: 附加到已运行的进程 (最常用)
gdb -p <pid>

# 方式 3: 核心转储文件
gdb ./app_gateway core.<pid>
```

### 4.3 多线程调试核心命令

#### 查看线程信息

```gdb
# 列出所有线程
(gdb) info threads

# 输出示例:
#   Id   Target Id          Frame
# * 1    Thread 0x7f... (LWP 12345) "app_gateway" main () at main.cpp:336
#   2    Thread 0x7f... (LWP 12346) "app_gateway" 0x00007f... in select () at serial_bus.cpp:176
#   3    Thread 0x7f... (LWP 12347) "app_gateway" 0x00007f... in futex_wait () at polling_manager.cpp:189
#   ...

# 切换到指定线程
(gdb) thread 2

# 切换到指定线程并查看调用栈
(gdb) thread apply 2 bt

# 查看所有线程的调用栈
(gdb) thread apply all bt
```

#### 查看调用栈

```gdb
# 当前线程的调用栈
(gdb) bt

# 带局部变量的调用栈
(gdb) bt full

# 输出示例 (死锁场景):
# #0  0x00007f... in futex_wait_cancelable () from /lib64/libpthread.so.0
# #1  0x00007f... in __pthread_mutex_lock_full () from /lib64/libpthread.so.0
# #2  0x00005555... in SerialBus::close() () at serial_bus.cpp:137
# #3  0x00005555... in main() () at main.cpp:399
```

#### 查看锁的状态

```gdb
# 查看 std::mutex 的内部状态
(gdb) print mtx_
# $1 = {<std::__mutex_base> = {_M_mutex = {__data = {__lock = 2, __owner = 12347, ...}}}}

# __owner 字段表示持有锁的线程 ID (LWP)
# __lock = 0: 未锁定
# __lock = 1: 已锁定, 无等待者
# __lock = 2: 已锁定, 有等待者
```

#### 查看变量

```gdb
# 查看变量值
(gdb) print g_web_running
(gdb) print fd_
(gdb) print running_

# 查看原子变量
(gdb) print running_._M_i

# 查看 this 指针
(gdb) print *this

# 查看数组
(gdb) print *threads_._M_impl._M_start @ threads_.size()
```

### 4.4 断点与调试

```gdb
# 在函数入口打断点
(gdb) break SerialBus::close
(gdb) break SerialBus::transact

# 在指定文件:行打断点
(gdb) break serial_bus.cpp:137
(gdb) break main.cpp:392

# 条件断点: 只在特定条件下暂停
(gdb) break serial_bus.cpp:260 if fd_ < 0

# 观察点: 变量被修改时暂停
(gdb) watch g_web_running
(gdb) watch fd_

# 捕获信号
(gdb) catch signal SIGINT

# 忽略断点 N 次
(gdb) ignore 1 10

# 继续执行
(gdb) continue

# 单步 (进入函数)
(gdb) step

# 单步 (不进入函数)
(gdb) next

# 执行到当前函数返回
(gdb) finish
```

### 4.5 附加到运行中进程的完整流程

```bash
# 1. 找到进程 PID
pidof app_gateway

# 2. 附加 GDB (会暂停进程)
gdb -p <pid>

# 3. 在 GDB 中查看所有线程
(gdb) info threads

# 4. 查看每个线程在做什么
(gdb) thread apply all bt

# 5. 找到可疑线程, 切换过去
(gdb) thread 3

# 6. 查看详细调用栈和变量
(gdb) bt full

# 7. 检查锁的状态
(gdb) print mtx_

# 8. 继续执行或退出
(gdb) continue   # 继续运行
(gdb) detach     # 分离进程 (不终止)
(gdb) quit       # 退出 GDB (会终止进程)
```

---

## 5. 常见多线程问题的 GDB 排查流程

### 5.1 排查死锁

**场景：** 程序挂起，CPU 使用率为 0

**GDB 排查步骤：**

```gdb
# 1. 附加到进程
(gdb) -p <pid>

# 2. 查看所有线程状态
(gdb) info threads
# 注意: 多个线程停在 futex_wait 通常是死锁

# 3. 查看每个线程的调用栈
(gdb) thread apply all bt

# 4. 分析锁的持有关系
# 线程 A: 持有锁1, 等待锁2
# 线程 B: 持有锁2, 等待锁1
# → 经典死锁

# 5. 查看锁的 __owner 字段确认持有者
(gdb) thread 2
(gdb) print mtx_
# $1 = {..., __owner = 12347, ...}
# → 线程 12347 持有此锁

(gdb) thread 3
(gdb) print other_mtx_
# $2 = {..., __owner = 12346, ...}
# → 线程 12346 持有此锁
```

**本次项目中的死锁：**

```
线程 1 (main): 持有 env_.mtx → 调用 env_ = EnvDataCpp{} → 尝试再次获取 env_.mtx → 死锁!
```

GDB 显示：
```
Thread 1 (LWP 12345):
#0  futex_wait_cancelable
#1  __pthread_mutex_lock_full
#2  EnvDataCpp::operator= (env_.mtx 已被持有)
#3  DevHumidifier::init()
#4  init_all_devices()
#5  main()
```

### 5.2 排查线程阻塞

**场景：** 程序响应慢，但不死锁

```gdb
# 1. 查看线程状态
(gdb) info threads
# 输出: Thread 3 ... "SerialBus" select () at serial_bus.cpp:176
# → 线程阻塞在 select() 调用

# 2. 查看阻塞线程的调用栈
(gdb) thread 3
(gdb) bt
# #0  select () at serial_bus.cpp:176
# #1  rawRecv () at serial_bus.cpp:176
# #2  transact () at serial_bus.cpp:294
# #3  pollGroupOnce () at polling_manager.cpp:213
# #4  pollGroupThread () at polling_manager.cpp:176

# 3. 查看 select 的超时参数
(gdb) frame 0
(gdb) print tv
# $1 = {tv_sec = 1, tv_usec = 0}
# → 超时 1 秒, 正常

# 4. 查看 fd 是否有效
(gdb) frame 2
(gdb) print fd_
# $2 = 3
# → fd 有效
```

### 5.3 排查信号处理

**场景：** Ctrl+C 不生效

```gdb
# 1. 设置信号断点
(gdb) catch signal SIGINT

# 2. 继续运行, 按 Ctrl+C
(gdb) continue

# 3. 信号触发后查看当前线程
(gdb) info threads
# → 看信号被投递到哪个线程

# 4. 如果信号没到达主线程, 说明被其他线程捕获
# 解决: 在创建线程前用 sigprocmask 阻塞信号, 主线程中解除阻塞

# 5. 检查信号处理函数
(gdb) print signal_handler
(gdb) break signal_handler
(gdb) continue
# → 确认处理函数是否被调用
```

### 5.4 排查段错误 (Segmentation Fault)

**场景：** 程序崩溃

```bash
# 1. 启用核心转储
ulimit -c unlimited

# 2. 运行程序, 等待崩溃
./app_gateway

# 3. 用 GDB 分析核心转储
gdb ./app_gateway core.<pid>
```

```gdb
# 查看崩溃位置
(gdb) bt
# #0  0x00005555... in DevSmoke::procState () at dev_smoke.cpp:45
# #1  0x00005555... in PollingManager::pollGroupOnce () at polling_manager.cpp:217

# 查看崩溃点的变量
(gdb) frame 0
(gdb) print resp
# $1 = 0x0  ← 空指针!

# 查看调用者传入的参数
(gdb) frame 1
(gdb) print task
(gdb) print resp
```

### 5.5 排查数据竞争

**场景：** 偶发的数值异常或崩溃

```bash
# 使用 ThreadSanitizer 编译
g++ -fsanitize=thread -g -o app_gateway ...

# 运行程序
./app_gateway
# TSan 会输出类似:
# WARNING: ThreadSanitizer: data race
#   Read at 0x... by thread 2:
#     DevSmoke::procState() at dev_smoke.cpp:35
#   Previous write at 0x... by thread 5:
#     DevSmoke::procState() at dev_smoke.cpp:35
```

---

## 6. 本次项目排错复盘

### 6.1 问题一：EnvDataCpp 死锁

**现象：** 程序启动后网页打不开，日志停在"所有设备已初始化"之后

**排查过程：**
1. 检查日志 → 发现日志停在 `init_all_devices()` 之后
2. 怀疑初始化阶段有死锁
3. 用 `gdb -p <pid>` 附加到进程
4. `info threads` → 主线程停在 `futex_wait`
5. `bt` → 主线程在 `__pthread_mutex_lock_full` → `EnvDataCpp::operator=`
6. 分析：`init()` 中先 `lock_guard(env_.mtx)`，再 `env_ = EnvDataCpp{}`，赋值运算符内部再次 `scoped_lock(mtx)` → 非递归 mutex 死锁

**根因：** `std::mutex` 不可重入，同一线程二次加锁死锁

**修复：** 移除外层 `lock_guard`

**面试回答要点：**
> "程序启动后网页无法访问，通过日志分析发现初始化阶段卡住。用 GDB 附加到进程，查看线程调用栈发现主线程阻塞在 mutex 加锁操作。分析代码发现 `init()` 函数中先用 `lock_guard` 锁定了 `env_.mtx`，然后调用赋值运算符，赋值运算符内部再次用 `scoped_lock` 尝试加锁同一个 mutex。由于 `std::mutex` 不可重入，同一线程二次加锁导致死锁。修复方案是移除外层 `lock_guard`，因为赋值运算符内部的 `scoped_lock` 已经提供了足够的保护。"

### 6.2 问题二：serialBus.close() 阻塞 30 秒

**现象：** Ctrl+C 后程序 30+ 秒才退出，日志显示"正在关闭串口"到"正在停止轮询"之间有 29.5 秒间隔

**排查过程：**
1. 检查日志时间戳 → `serialBus.close()` 耗时 29.5 秒
2. 分析 `close()` 代码 → 需要获取 `recursive_mutex`
3. 分析 `transact()` 代码 → 整个事务期间（包括 I/O）持有 mutex
4. 推理：轮询线程在 `rawRecv()` 的 `select()` 中阻塞，持有 mutex → `close()` 等待所有轮询线程释放 mutex → 20 个线程 × ~1.5 秒/线程 ≈ 30 秒

**GDB 验证：**
```gdb
(gdb) info threads
# Thread 1: futex_wait → SerialBus::close()
# Thread 3: select → rawRecv → transact → pollGroupOnce
# Thread 4: select → rawRecv → transact → pollGroupOnce
# ...

(gdb) thread 1
(gdb) print mtx_
# {__lock = 2, __owner = 12346, ...}  ← Thread 3 持有锁
```

**根因：** `transact()` 在 I/O 期间持有 mutex，`close()` 必须等待所有 I/O 完成

**修复：**
1. `transact()` 中 I/O 操作不持锁
2. 添加 shutdown pipe，`rawRecv()` 的 `select()` 同时监听管道
3. `close()` 写管道唤醒阻塞的 `select()`

**面试回答要点：**
> "退出时 `serialBus.close()` 阻塞了 30 秒。通过日志时间线分析，定位到 `close()` 需要获取 mutex，但轮询线程在 I/O 期间持有 mutex。用 GDB 验证：附加到进程后查看线程状态，主线程阻塞在 `futex_wait`（等待 mutex），轮询线程阻塞在 `select()`（等待串口数据），且轮询线程是 mutex 的持有者。
>
> 解决方案有三个层面：
> 1. **释放 I/O 锁**：将 `transact()` 中的 mutex 保护范围缩小，不在 I/O 期间持锁
> 2. **Shutdown Pipe**：在 `SerialBus` 中添加管道，`rawRecv()` 的 `select()` 同时监听管道，`close()` 写管道唤醒阻塞线程
> 3. **清理顺序优化**：先关闭串口（触发管道唤醒），再停止轮询器"

### 6.3 问题三：Ctrl+C 信号处理

**现象：** 多线程程序中 Ctrl+C 无法终止程序

**排查过程：**
1. 检查信号注册代码 → 使用 `sigaction`，未设置 `SA_RESTART`
2. 在信号处理函数中加 `write(STDERR_FILENO, ...)` 确认是否被调用
3. 发现信号被调用，但主循环的 `usleep()` 未被中断
4. 改用 `select()` + self-pipe trick

**根因：** `usleep()` 在某些实现中不被信号中断；多线程中信号可能被非主线程接收

**修复：**
1. 使用 self-pipe trick：信号处理函数写管道，主循环 `select()` 监听管道
2. `select()` 超时设为 1 秒（响应延迟 < 1 秒）
3. `sigaction` 不设置 `SA_RESTART`

**面试回答要点：**
> "多线程程序中信号处理有几个坑：
> 1. 信号会被投递到任意未阻塞的线程，不一定是主线程
> 2. `usleep()` 在某些实现中不被信号中断
> 3. `SA_RESTART` 会让被中断的系统调用自动重启
>
> 解决方案是 self-pipe trick：信号处理函数只做两件事——设置退出标志和写管道。主循环用 `select()` 监听管道，超时 1 秒。这样无论信号被哪个线程接收，管道写入都会唤醒主循环的 `select()`。"

---

## 7. 面试高频问题与回答要点

### Q1: 如何排查多线程程序中的死锁？

**回答框架：**
1. **现象识别**：程序挂起，CPU 使用率为 0
2. **快速定位**：`gdb -p <pid>` → `info threads` → `thread apply all bt`
3. **分析锁的持有关系**：查看 `__owner` 字段，画出锁依赖图
4. **根因分类**：
   - 同一线程对非递归 mutex 二次加锁（本次项目的 EnvDataCpp 问题）
   - 不同线程以不同顺序获取多个锁（经典 ABBA 死锁）
   - 持有锁时调用外部函数，外部函数也尝试获取同一锁
5. **预防措施**：
   - 使用 `std::recursive_mutex` 允许重入
   - 统一锁的获取顺序
   - 缩小临界区范围
   - 使用 `std::lock()` 同时获取多个锁

### Q2: 如何实现多线程程序的优雅退出？

**回答框架：**
1. **信号处理**：使用 self-pipe trick，信号处理函数写管道
2. **主循环**：`select()` 监听管道 + 超时，检测退出标志
3. **清理顺序**：先停止接受新任务 → 停止工作线程 → 关闭资源
4. **中断阻塞 I/O**：使用 shutdown pipe 或 `pthread_kill(thread, SIGUSR1)`
5. **超时保护**：`pthread_join()` 设置超时，超时后 `detach()` 避免永久阻塞

### Q3: `std::mutex` 和 `std::recursive_mutex` 的区别？

**回答要点：**
- `std::mutex`：不可重入，同一线程二次加锁是 **undefined behavior**（通常死锁）
- `std::recursive_mutex`：可重入，同一线程可以多次加锁，但必须解锁相同次数
- 性能：`std::mutex` 略快（无需维护重入计数）
- 选择：如果需要在持有锁的函数中调用另一个也需要同一把锁的函数，用 `recursive_mutex`；否则用 `mutex` 更安全（更容易发现设计问题）

### Q4: 如何用 GDB 调试已经运行中的多线程程序？

**回答要点：**
```bash
# 1. 编译时加 -g -O0
# 2. 附加到进程
gdb -p <pid>
# 3. 查看所有线程
info threads
# 4. 查看每个线程在做什么
thread apply all bt
# 5. 切换到可疑线程深入分析
thread <id>
bt full
# 6. 检查锁的状态
print mutex_variable
# 7. detach 或继续
detach
```

### Q5: 如何避免多线程中的数据竞争？

**回答要点：**
1. **识别共享数据**：哪些数据被多个线程读写？
2. **选择同步机制**：
   - 简单类型（int, bool, 指针）→ `std::atomic`
   - 复合类型（结构体、多个相关字段）→ `std::mutex`
   - 读多写少 → `std::shared_mutex`（读写锁）
3. **RAII 管理锁**：`std::lock_guard` / `std::unique_lock`，防止忘记解锁
4. **工具检测**：ThreadSanitizer (`-fsanitize=thread`) 可以在运行时检测数据竞争
5. **设计原则**：尽量减少共享状态，优先使用消息传递而非共享内存

### Q6: Linux 中 `close(fd)` 会中断另一个线程的 `select()` 吗？

**回答要点：**
- **不会可靠中断**。POSIX 标准未定义此行为
- Linux 上，`close(fd)` 不会唤醒阻塞在 `select()` 上的另一个线程
- 如果 fd 被关闭，`select()` 可能返回 EBADF，也可能不返回（行为不确定）
- **正确做法**：使用 self-pipe / eventfd / `pthread_kill(thread, SIGUSR1)` 来中断阻塞的 `select()`

---

## 附录 A：常用 GDB 命令速查表

| 命令 | 缩写 | 说明 |
|------|------|------|
| `run` | `r` | 启动程序 |
| `continue` | `c` | 继续执行 |
| `next` | `n` | 单步（不进入函数） |
| `step` | `s` | 单步（进入函数） |
| `finish` | | 执行到当前函数返回 |
| `break func` | `b func` | 在函数入口打断点 |
| `break file:line` | `b file:line` | 在指定行打断点 |
| `watch var` | | 变量被修改时暂停 |
| `info breakpoints` | `i b` | 列出所有断点 |
| `delete <n>` | `d <n>` | 删除断点 |
| `backtrace` | `bt` | 查看调用栈 |
| `bt full` | | 调用栈 + 局部变量 |
| `frame <n>` | `f <n>` | 切换到指定栈帧 |
| `print expr` | `p expr` | 打印表达式 |
| `info threads` | `i th` | 列出所有线程 |
| `thread <n>` | `t <n>` | 切换到指定线程 |
| `thread apply all bt` | | 所有线程的调用栈 |
| `set var = expr` | | 修改变量值 |
| `detach` | | 分离进程（不终止） |
| `quit` | `q` | 退出 GDB |

## 附录 B：常用排错命令速查表

```bash
# 查看进程的线程
ps -eLf | grep <pid>

# 查看进程打开的文件描述符
ls -la /proc/<pid>/fd/

# 查看进程的信号掩码
cat /proc/<pid>/status | grep -E 'Sig|Shd'

# 查看端口占用
ss -tlnp | grep <port>

# 实时跟踪系统调用
strace -p <pid> -e trace=read,write,select,futex,mmap

# 查看 mutex 的 owner
gdb -p <pid> -batch -ex 'print mutex_variable'

# ThreadSanitizer 编译
g++ -fsanitize=thread -g -o app ...

# Valgrind 线程检查
valgrind --tool=helgrind ./app

# 查看进程的锁持有情况 (需要 gdb)
gdb -p <pid> -batch -ex 'info threads' -ex 'thread apply all bt'
```
