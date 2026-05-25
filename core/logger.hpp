/**
 * @file logger.hpp
 * @brief 线程安全的日志管理系统
 *
 * 特性:
 *   - 每次运行生成独立日志文件 (带时间戳)
 *   - 日志文件上限50个, 自动删除最旧的
 *   - 支持 INFO / WARNING / ERROR 三个级别
 *   - 线程安全 (多线程同时写日志)
 *   - 同时输出到文件和控制台
 *   - 单例模式, 全局唯一
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>
#include <cstdint>
#include <cstdarg>

enum class LogLevel {
    INFO    = 0,
    WARNING = 1,
    ERROR_  = 2   // ERROR 在 Windows 头文件中有宏定义, 加下划线避免冲突
};

class Logger {
public:
    /**
     * @brief 获取全局唯一实例
     */
    static Logger &instance();

    /**
     * @brief 初始化日志系统
     * @param logDir     日志文件存放目录 (默认 "./logs")
     * @param maxFiles   最大日志文件数 (默认 50)
     * @param minLevel   最低输出级别 (默认 INFO)
     * @return 0=成功, -1=失败
     */
    int init(const std::string &logDir = "./logs",
             int maxFiles = 50,
             LogLevel minLevel = LogLevel::INFO);

    /**
     * @brief 关闭日志系统
     */
    void shutdown();

    /**
     * @brief 写入日志 (C printf 风格)
     * @param level  日志级别
     * @param file   源文件名 (__FILE__)
     * @param line   行号 (__LINE__)
     * @param fmt    格式化字符串
     */
    void log(LogLevel level, const char *file, int line, const char *fmt, ...);

    /**
     * @brief 写入日志 (C++ string)
     */
    void log(LogLevel level, const char *file, int line, const std::string &msg);

    /**
     * @brief 便捷宏用的内部方法 (带可变参数)
     */
    void logV(LogLevel level, const char *file, int line, const char *fmt, va_list args);

    /**
     * @brief 设置最低输出级别
     */
    void setMinLevel(LogLevel level) { minLevel_ = level; }

    /**
     * @brief 获取当前日志文件路径
     */
    const std::string &getCurrentLogFile() const { return currentLogFile_; }

    /**
     * @brief 是否已初始化
     */
    bool isInitialized() const { return initialized_; }

private:
    Logger();
    ~Logger();
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    /**
     * @brief 创建日志文件名 (带时间戳)
     */
    std::string createLogFileName() const;

    /**
     * @brief 清理旧日志文件, 保持在 maxFiles_ 以内
     */
    void cleanOldLogs();

    /**
     * @brief 级别转字符串
     */
    static const char *levelStr(LogLevel level);

    /**
     * @brief 获取当前时间字符串
     */
    std::string currentTimeStr() const;

    /**
     * @brief 获取简短文件名 (去掉路径前缀)
     */
    static const char *shortFileName(const char *file);

    std::string   logDir_;
    int           maxFiles_;
    LogLevel      minLevel_;
    bool          initialized_ = false;
    std::ofstream fileStream_;
    std::string   currentLogFile_;
    std::mutex    mtx_;
};

/* ============================================================
 * 便捷日志宏
 * 用法:
 *   LOG_INFO("设备 %s 温度: %.1f°C", name, temp);
 *   LOG_WARNING("轮询超时: 设备地址 0x%02X", addr);
 *   LOG_ERROR("串口打开失败: %s", device);
 * ============================================================ */
#define LOG_INFO(fmt, ...)  \
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARNING(fmt, ...) \
    Logger::instance().log(LogLevel::WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    Logger::instance().log(LogLevel::ERROR_, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* LOGGER_HPP */