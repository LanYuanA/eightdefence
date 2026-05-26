/**
 * @file app_logger.cpp
 * @brief 应用独立日志器实现
 */

#include "app_logger.hpp"
#include "logger.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <sys/stat.h>

AppLogger::AppLogger(const std::string& appName, const std::string& logDir)
    : appName_(appName), logDir_(logDir) {}

AppLogger::~AppLogger() {
    shutdown();
}

int AppLogger::init() {
    std::lock_guard<std::mutex> lock(mtx_);

    if (initialized_) return 0;

    // 确保主日志目录存在
    struct stat st;
    if (stat(logDir_.c_str(), &st) != 0) {
        if (mkdir(logDir_.c_str(), 0755) != 0) {
            std::cerr << "[AppLogger] 创建日志目录失败: " << logDir_ << "\n";
            return -1;
        }
    }

    // 创建应用专属子目录: logs/{appName}/
    appDir_ = logDir_ + "/" + appName_;
    if (stat(appDir_.c_str(), &st) != 0) {
        if (mkdir(appDir_.c_str(), 0755) != 0) {
            std::cerr << "[AppLogger] 创建应用日志目录失败: " << appDir_ << "\n";
            return -1;
        }
    }

    // 创建日志文件
    currentLogFile_ = appDir_ + "/" + createLogFileName();
    fileStream_.open(currentLogFile_, std::ios::out | std::ios::app);
    if (!fileStream_.is_open()) {
        std::cerr << "[AppLogger] 无法创建日志文件: " << currentLogFile_ << "\n";
        return -1;
    }

    initialized_ = true;

    // 写入日志头
    fileStream_ << "======================================================\n";
    fileStream_ << "  应用日志: " << appName_ << "\n";
    fileStream_ << "  启动时间: " << currentTimeStr() << "\n";
    fileStream_ << "  日志文件: " << currentLogFile_ << "\n";
    fileStream_ << "======================================================\n";
    fileStream_.flush();

    std::cout << "[AppLogger] " << appName_ << " 日志已启动: " << currentLogFile_ << "\n";
    return 0;
}

void AppLogger::shutdown() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (fileStream_.is_open()) {
        fileStream_ << "\n======================================================\n";
        fileStream_ << "  应用日志关闭: " << appName_ << " @ " << currentTimeStr() << "\n";
        fileStream_ << "======================================================\n";
        fileStream_.flush();
        fileStream_.close();
    }
    initialized_ = false;
}

void AppLogger::log(LogLevel level, const char *file, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logV(level, file, line, fmt, args);
    va_end(args);
}

void AppLogger::logV(LogLevel level, const char *file, int line, const char *fmt, va_list args) {
    // 格式化消息
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, args);

    // 构建日志行
    std::ostringstream oss;
    oss << currentTimeStr()
        << " [" << levelStr(level) << "] "
        << shortFileName(file) << ":" << line
        << " - " << buf;

    std::string logLine = oss.str();

    // 写入应用日志文件 (仅写入应用专属日志, 不写入主系统日志)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (fileStream_.is_open()) {
            fileStream_ << logLine << "\n";
            fileStream_.flush();
        }
    }
}

std::string AppLogger::createLogFileName() const {
    time_t now = time(nullptr);
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);

    char name[256];
    snprintf(name, sizeof(name), "%s_%04d%02d%02d_%02d%02d%02d.log",
             appName_.c_str(),
             tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
             tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec);
    return std::string(name);
}

std::string AppLogger::currentTimeStr() const {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    time_t sec = ts.tv_sec;
    int ms = (int)(ts.tv_nsec / 1000000);

    struct tm tm_buf;
    localtime_r(&sec, &tm_buf);

    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
             tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, ms);
    return std::string(buf);
}

const char *AppLogger::shortFileName(const char *file) {
    const char *p = strrchr(file, '/');
    return p ? p + 1 : file;
}

const char *AppLogger::levelStr(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR_:  return "ERROR";
        default:                return "UNKNOWN";
    }
}
