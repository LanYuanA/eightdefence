/**
 * @file app_logger.hpp
 * @brief 应用独立日志器
 *
 * 每个应用持有一个 AppLogger 实例，写日志时：
 *   1. 写入应用专属日志文件 (如 security_20260526_083813.log)
 *   2. 同时写入主系统日志 (通过 Logger::instance())
 *
 * 用法:
 *   // 在 AppBase 子类中 (m_logger 从 AppBase 继承)
 *   APP_LOG_INFO("初始化完成, 路由: %s", m_routePrefix.c_str());
 *   APP_LOG_ERROR("传感器读取失败: %s", err);
 */

#ifndef APP_LOGGER_HPP
#define APP_LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>

enum class LogLevel;

class AppLogger {
public:
    /**
     * @brief 构造函数
     * @param appName  应用名称 (用于日志文件名前缀, 如 "security")
     * @param logDir   日志目录 (默认 "./logs")
     */
    AppLogger(const std::string& appName, const std::string& logDir = "./logs");
    ~AppLogger();

    /**
     * @brief 初始化日志文件
     * @return 0=成功, -1=失败
     */
    int init();

    /**
     * @brief 关闭日志文件
     */
    void shutdown();

    /**
     * @brief 写入日志 (printf 风格)
     *        同时写入应用日志文件和主系统日志
     */
    void log(LogLevel level, const char *file, int line, const char *fmt, ...);

    /**
     * @brief 写入日志 (va_list 核心实现)
     */
    void logV(LogLevel level, const char *file, int line, const char *fmt, va_list args);

    /**
     * @brief 获取当前日志文件路径
     */
    const std::string& getCurrentLogFile() const { return currentLogFile_; }

    bool isInitialized() const { return initialized_; }

private:
    std::string createLogFileName() const;
    std::string currentTimeStr() const;
    static const char *shortFileName(const char *file);
    static const char *levelStr(LogLevel level);

    std::string   appName_;
    std::string   logDir_;
    std::string   appDir_;
    std::string   currentLogFile_;
    std::ofstream fileStream_;
    std::mutex    mtx_;
    bool          initialized_ = false;
};

/* ============================================================
 * 应用日志便捷宏
 * 用法 (在 AppBase 子类中):
 *   APP_LOG_INFO("传感器数据: %d", value);
 *   APP_LOG_WARNING("设备离线: %s", name);
 *   APP_LOG_ERROR("通信失败: %s", err);
 *
 * 前提: 子类中有 m_logger 成员 (从 AppBase 继承)
 * ============================================================ */
#define APP_LOG_INFO(fmt, ...)  \
    do { if (m_logger) m_logger->log(LogLevel::INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define APP_LOG_WARNING(fmt, ...) \
    do { if (m_logger) m_logger->log(LogLevel::WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define APP_LOG_ERROR(fmt, ...) \
    do { if (m_logger) m_logger->log(LogLevel::ERROR_, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#endif /* APP_LOGGER_HPP */
