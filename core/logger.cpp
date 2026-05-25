/**
 * @file logger.cpp
 * @brief 日志管理系统实现 (使用POSIX API, 兼容嵌入式系统)
 */

#include "logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

/* ============================================================
 * 单例
 * ============================================================ */
Logger &Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() {}

Logger::~Logger() {
    shutdown();
}

/* ============================================================
 * 初始化
 * ============================================================ */
int Logger::init(const std::string &logDir, int maxFiles, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(mtx_);

    if (initialized_) {
        std::cerr << "[LOGGER] 已经初始化过了, 忽略重复调用\n";
        return 0;
    }

    logDir_    = logDir;
    maxFiles_  = maxFiles;
    minLevel_  = minLevel;

    /* 创建日志目录 (POSIX mkdir) */
    struct stat st;
    if (stat(logDir_.c_str(), &st) != 0) {
        if (mkdir(logDir_.c_str(), 0755) != 0) {
            std::cerr << "[LOGGER] 创建日志目录失败: " << logDir_ << "\n";
            return -1;
        }
    }

    /* 清理旧日志 */
    cleanOldLogs();

    /* 创建新的日志文件 */
    currentLogFile_ = logDir_ + "/" + createLogFileName();
    fileStream_.open(currentLogFile_, std::ios::out | std::ios::app);
    if (!fileStream_.is_open()) {
        std::cerr << "[LOGGER] 无法创建日志文件: " << currentLogFile_ << "\n";
        return -1;
    }

    initialized_ = true;

    /* 写入日志头 */
    fileStream_ << "======================================================\n";
    fileStream_ << "  日志系统启动: " << currentTimeStr() << "\n";
    fileStream_ << "  日志文件: " << currentLogFile_ << "\n";
    fileStream_ << "  最大文件数: " << maxFiles_ << "\n";
    fileStream_ << "  最低级别: " << levelStr(minLevel_) << "\n";
    fileStream_ << "======================================================\n";
    fileStream_.flush();

    std::cout << "[LOGGER] 日志系统已启动: " << currentLogFile_ << "\n";
    return 0;
}

/* ============================================================
 * 关闭
 * ============================================================ */
void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (fileStream_.is_open()) {
        fileStream_ << "\n======================================================\n";
        fileStream_ << "  日志系统关闭: " << currentTimeStr() << "\n";
        fileStream_ << "======================================================\n";
        fileStream_.flush();
        fileStream_.close();
    }
    initialized_ = false;
}

/* ============================================================
 * 写日志 (printf 风格)
 * ============================================================ */
void Logger::log(LogLevel level, const char *file, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logV(level, file, line, fmt, args);
    va_end(args);
}

/* ============================================================
 * 写日志 (string)
 * ============================================================ */
void Logger::log(LogLevel level, const char *file, int line, const std::string &msg) {
    log(level, file, line, "%s", msg.c_str());
}

/* ============================================================
 * 写日志 (va_list 核心实现)
 * ============================================================ */
void Logger::logV(LogLevel level, const char *file, int line, const char *fmt, va_list args) {
    if (level < minLevel_) return;

    /* 格式化消息 */
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, args);

    /* 构建完整日志行 */
    std::ostringstream oss;
    oss << currentTimeStr()
        << " [" << levelStr(level) << "] "
        << shortFileName(file) << ":" << line
        << " - " << buf;

    std::string logLine = oss.str();

    /* 写入文件 + 控制台 (线程安全) */
    {
        std::lock_guard<std::mutex> lock(mtx_);

        if (fileStream_.is_open()) {
            fileStream_ << logLine << "\n";
            fileStream_.flush();
        }

        /* 控制台输出: 根据级别使用不同的输出流 */
        if (level >= LogLevel::ERROR_) {
            std::cerr << logLine << "\n";
        } else {
            std::cout << logLine << "\n";
        }
    }
}

/* ============================================================
 * 创建日志文件名 (格式: gateway_20260525_201630.log)
 * ============================================================ */
std::string Logger::createLogFileName() const {
    time_t now = time(nullptr);
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);

    char name[128];
    snprintf(name, sizeof(name), "gateway_%04d%02d%02d_%02d%02d%02d.log",
             tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
             tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec);
    return std::string(name);
}

/* ============================================================
 * 清理旧日志: 按文件名排序, 删除最旧的超出限制的文件
 * ============================================================ */
void Logger::cleanOldLogs() {
    DIR *dir = opendir(logDir_.c_str());
    if (!dir) return;

    /* 收集所有 .log 文件 */
    std::vector<std::string> logFiles;
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
            std::string name(entry->d_name);
            if (name.size() > 4 && name.substr(name.size() - 4) == ".log") {
                logFiles.push_back(name);
            }
        }
    }
    closedir(dir);

    /* 按文件名排序 (文件名含时间戳, 字典序=时间序) */
    std::sort(logFiles.begin(), logFiles.end());

    /* 删除超出上限的最旧文件 */
    int excess = (int)logFiles.size() - maxFiles_ + 1;  // 留一个位置给新文件
    for (int i = 0; i < excess && i < (int)logFiles.size(); i++) {
        std::string fullPath = logDir_ + "/" + logFiles[i];
        if (unlink(fullPath.c_str()) == 0) {
            std::cout << "[LOGGER] 已清理旧日志: " << logFiles[i] << "\n";
        }
    }
}

/* ============================================================
 * 级别字符串
 * ============================================================ */
const char *Logger::levelStr(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR_:  return "ERROR";
        default:                return "UNKNOWN";
    }
}

/* ============================================================
 * 当前时间字符串
 * ============================================================ */
std::string Logger::currentTimeStr() const {
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

/* ============================================================
 * 简短文件名 (去掉路径前缀, 只保留文件名)
 * ============================================================ */
const char *Logger::shortFileName(const char *file) {
    const char *p = strrchr(file, '/');
    return p ? p + 1 : file;
}