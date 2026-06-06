/**
 * @file serial_bus.cpp
 * @brief 线程安全的RS-485串口总线管理器实现
 */

#include "serial_bus.hpp"
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <algorithm>

extern "C" {
#include "modbus_core.h"
}

/* ============================================================
 * 内部工具: HEX转换 (复用 modbus_core.c 中的逻辑)
 * ============================================================ */
static int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int hex_to_bytes(const char *hex, uint8_t *out, size_t out_cap, size_t *out_len) {
    size_t hex_len = strlen(hex);
    size_t j = 0;
    if (hex_len == 0 || (hex_len % 2) != 0) return -1;
    if (hex_len / 2 > out_cap) return -2;
    for (size_t i = 0; i < hex_len; i += 2) {
        int hi = hex_char_to_val(hex[i]);
        int lo = hex_char_to_val(hex[i + 1]);
        if (hi < 0 || lo < 0) return -3;
        out[j++] = static_cast<uint8_t>((hi << 4) | lo);
    }
    *out_len = j;
    return 0;
}

static speed_t baud_to_termios(int baud) {
    switch (baud) {
        case 1200:   return B1200;
        case 2400:   return B2400;
        case 4800:   return B4800;
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     return 0;
    }
}

/* ============================================================
 * 构造/析构
 * ============================================================ */
SerialBus::SerialBus(const char *device, int baud)
    : device_(device), baud_(baud) {
    pipe(shutdown_pipe_);
    fcntl(shutdown_pipe_[0], F_SETFL, O_NONBLOCK);
    fcntl(shutdown_pipe_[1], F_SETFL, O_NONBLOCK);
}

SerialBus::~SerialBus() {
    close();
    if (shutdown_pipe_[0] >= 0) { ::close(shutdown_pipe_[0]); shutdown_pipe_[0] = -1; }
    if (shutdown_pipe_[1] >= 0) { ::close(shutdown_pipe_[1]); shutdown_pipe_[1] = -1; }
}

/* ============================================================
 * 打开/关闭串口
 * ============================================================ */
int SerialBus::open() {
    std::unique_lock<std::shared_mutex> lock(mtx_);

    if (fd_ >= 0) {
        return 0;  // 已经打开
    }

    speed_t speed = baud_to_termios(baud_);
    if (speed == 0) {
        fprintf(stderr, "[SerialBus] 不支持的波特率: %d\n", baud_);
        return -1;
    }

    fd_ = ::open(device_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd_ < 0) {
        fprintf(stderr, "[SerialBus] 打开串口失败: %s (%s)\n",
                device_.c_str(), strerror(errno));
        return -2;
    }

    struct termios tty;
    if (tcgetattr(fd_, &tty) != 0) {
        fprintf(stderr, "[SerialBus] 获取串口属性失败: %s\n", strerror(errno));
        ::close(fd_);
        fd_ = -1;
        return -3;
    }

    // 清除所有输入/输出标志
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

    // 设置波特率
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // 8N1, 无流控
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif

    // 非阻塞模式: VMIN=0, VTIME=0
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        fprintf(stderr, "[SerialBus] 设置串口属性失败: %s\n", strerror(errno));
        ::close(fd_);
        fd_ = -1;
        return -4;
    }

    tcflush(fd_, TCIOFLUSH);

    printf("[SerialBus] 串口已打开: %s @ %d baud\n", device_.c_str(), baud_);
    return 0;
}

void SerialBus::close() {
    // 先写管道唤醒所有阻塞在 select() 上的线程
    if (shutdown_pipe_[1] >= 0) {
        char c = 'X';
        write(shutdown_pipe_[1], &c, 1);
    }
    std::unique_lock<std::shared_mutex> lock(mtx_);
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
        printf("[SerialBus] 串口已关闭: %s\n", device_.c_str());
    }
}

/* ============================================================
 * 内部: 发送/接收/刷新
 * ============================================================ */
int SerialBus::rawSend(const uint8_t *data, size_t len) {
    int fd = fd_;
    ssize_t written = write(fd, data, len);
    if (written < 0) {
        fprintf(stderr, "[SerialBus] 发送失败: %s\n", strerror(errno));
        return -1;
    }
    if (static_cast<size_t>(written) != len) {
        fprintf(stderr, "[SerialBus] 发送不完整: %zd/%zu\n", written, len);
        return -2;
    }
    tcdrain(fd);  // 等待数据发送完成
    stats_.totalBytesSent += len;
    return 0;
}

int SerialBus::rawRecv(uint8_t *buf, size_t buf_cap, size_t *recv_len, int timeout_ms) {
    int fd = fd_;
    int pipe_fd = shutdown_pipe_[0];
    int nfds = (fd > pipe_fd ? fd : pipe_fd) + 1;
    size_t total_read = 0;

    while (total_read < buf_cap) {
        fd_set readfds;
        struct timeval tv;

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        if (pipe_fd >= 0) FD_SET(pipe_fd, &readfds);

        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int sel = select(nfds, &readfds, NULL, NULL, &tv);
        if (sel < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[SerialBus] select失败: %s\n", strerror(errno));
            return -1;
        }
        if (sel == 0) {
            break;  // 超时
        }

        // shutdown pipe 可读 → 正在关闭
        if (pipe_fd >= 0 && FD_ISSET(pipe_fd, &readfds)) {
            return -1;
        }

        ssize_t n = read(fd, buf + total_read, buf_cap - total_read);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            fprintf(stderr, "[SerialBus] 读取失败: %s\n", strerror(errno));
            return -2;
        }
        if (n == 0) break;

        total_read += static_cast<size_t>(n);

        // Modbus RTU: 如果已经收到数据，短暂等待看是否还有更多数据
        // 使用一个小的超时来检测帧结束
        if (total_read > 0) {
            fd_set checkfds;
            struct timeval short_tv;
            FD_ZERO(&checkfds);
            FD_SET(fd, &checkfds);
            if (pipe_fd >= 0) FD_SET(pipe_fd, &checkfds);
            short_tv.tv_sec = 0;
            short_tv.tv_usec = 50000;  // 50ms 无新数据则认为帧结束

            int s2 = select(nfds, &checkfds, NULL, NULL, &short_tv);
            if (s2 <= 0) break;  // 超时或错误，认为帧结束
            if (pipe_fd >= 0 && FD_ISSET(pipe_fd, &checkfds)) break;
        }
    }

    *recv_len = total_read;
    stats_.totalBytesRecv += total_read;
    return 0;
}

void SerialBus::flushBuffers() {
    if (fd_ >= 0) {
        tcflush(fd_, TCIOFLUSH);
    }
}

void SerialBus::updateLatencyStats(double latency_ms) {
    // 使用指数移动平均
    double old_avg = stats_.avgLatencyMs.load();
    double new_avg = old_avg * 0.9 + latency_ms * 0.1;
    stats_.avgLatencyMs.store(new_avg);

    // 更新最大延迟
    double old_max = stats_.maxLatencyMs.load();
    while (latency_ms > old_max) {
        if (stats_.maxLatencyMs.compare_exchange_weak(old_max, latency_ms)) {
            break;
        }
    }
}

/* ============================================================
 * 事务接口
 * ============================================================ */
int SerialBus::transact(const uint8_t *request, size_t request_len,
                        uint8_t *response, size_t response_cap,
                        size_t *response_len, int timeout_ms,
                        bool isWrite) {
    auto start = std::chrono::steady_clock::now();

    // 持锁覆盖整个事务: RS-485 是半双工总线, send+recv 必须原子
    // 写操作用独占锁 (优先), 读操作用共享锁 (写等待时让步)
    auto lock_start = std::chrono::steady_clock::now();
    std::unique_lock<std::shared_mutex> write_lock;
    std::shared_lock<std::shared_mutex> read_lock;
    if (isWrite) {
        write_lock = std::unique_lock<std::shared_mutex>(mtx_);
    } else {
        read_lock = std::shared_lock<std::shared_mutex>(mtx_);
    }
    auto lock_end = std::chrono::steady_clock::now();
    double lock_wait_ms = std::chrono::duration<double, std::milli>(lock_end - lock_start).count();

    if (lock_wait_ms > 100.0) {
        stats_.busContentionCount++;
    }
    if (isWrite && lock_wait_ms > 10.0) {
        stats_.writePreemptCount++;
    }

    stats_.totalTransactions++;

    if (fd_ < 0) {
        if (auto_reconnect_) {
            int rc = const_cast<SerialBus *>(this)->open();
            if (rc != 0) {
                stats_.totalErrors++;
                return -100;
            }
        } else {
            stats_.totalErrors++;
            return -101;
        }
    }

    // 帧间延时 (Modbus RTU 3.5字符时间)
    if (inter_frame_delay_us_ > 0) {
        usleep(inter_frame_delay_us_);
    }

    // 清空残留数据
    tcflush(fd_, TCIOFLUSH);

    // 发送请求
    int rc = rawSend(request, request_len);
    if (rc != 0) {
        stats_.totalErrors++;
        if (auto_reconnect_) {
            ::close(fd_);
            fd_ = -1;
        }
        return -200;
    }

    // 接收响应
    rc = rawRecv(response, response_cap, response_len, timeout_ms);
    if (rc != 0) {
        stats_.totalErrors++;
        return -300;
    }

    if (*response_len == 0) {
        stats_.totalErrors++;
        return -400;  // 超时无响应
    }

    // 更新延迟统计
    auto end = std::chrono::steady_clock::now();
    double latency = std::chrono::duration<double, std::milli>(end - start).count();
    updateLatencyStats(latency);

    return 0;
}

int SerialBus::transactHex(const char *hex_cmd,
                           uint8_t *response, size_t response_cap,
                           size_t *response_len, int timeout_ms,
                           bool isWrite) {
    uint8_t request[512];
    size_t request_len = 0;

    // 解析HEX命令
    int rc = hex_to_bytes(hex_cmd, request, sizeof(request), &request_len);
    if (rc != 0) return -10;

    // 自动追加CRC16
    if (request_len + 2 <= sizeof(request)) {
        uint16_t crc = crc16_modbus(request, request_len);
        request[request_len]     = crc & 0xFF;
        request[request_len + 1] = (crc >> 8) & 0xFF;
        request_len += 2;
    } else {
        return -11;
    }

    return transact(request, request_len, response, response_cap, response_len, timeout_ms, isWrite);
}