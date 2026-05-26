/**
 * @file svc_command_center.cpp
 * @brief 指挥中心报警服务实现
 */

#include "svc_command_center.hpp"
#include "core/logger.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

void SvcCommandCenter::activate() {
    active_.store(true);
    printf("  => [SvcCommandCenter] 指挥中心报警服务已激活\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[指挥中心报警] 服务已激活");
}

void SvcCommandCenter::deactivate() {
    if (!active_.exchange(false)) return;
    clearAlerts();
    printf("  => [SvcCommandCenter] 指挥中心报警服务已停止\n");
    Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__,
        "[指挥中心报警] 服务已停止, 告警已清除");
}

void SvcCommandCenter::alert(const std::string& level, const std::string& title, const std::string& message) {
    if (!active_.load()) return;

    AlertEntry entry;
    entry.level = level;
    entry.title = title;
    entry.message = message;
    entry.timestamp = now();

    {
        std::lock_guard<std::mutex> lock(mtx_);
        alerts_.push_back(std::move(entry));
        if (alerts_.size() > 100) {
            alerts_.erase(alerts_.begin(), alerts_.begin() + 20);
        }
    }

    // 写入主日志
    LogLevel logLevel = LogLevel::WARNING;
    if (level == "high") logLevel = LogLevel::ERROR_;
    else if (level == "low") logLevel = LogLevel::INFO;

    Logger::instance().log(logLevel, __FILE__, __LINE__,
        "[指挥中心报警] [%s] %s: %s", level.c_str(), title.c_str(), message.c_str());

    printf("  => [SvcCommandCenter] 告警 [%s] %s: %s\n",
           level.c_str(), title.c_str(), message.c_str());
}

void SvcCommandCenter::clearAlerts() {
    std::lock_guard<std::mutex> lock(mtx_);
    alerts_.clear();
}

std::vector<AlertEntry> SvcCommandCenter::getAlerts() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return alerts_;
}

std::string SvcCommandCenter::getAlertsJson() const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string json = "[";
    int count = 0;
    for (int i = static_cast<int>(alerts_.size()) - 1; i >= 0 && count < 50; i--, count++) {
        if (count > 0) json += ",";
        json += "{\"level\":\"" + alerts_[i].level + "\","
                "\"title\":\"" + alerts_[i].title + "\","
                "\"message\":\"" + alerts_[i].message + "\","
                "\"timestamp\":\"" + alerts_[i].timestamp + "\"}";
    }
    json += "]";
    return json;
}

std::string SvcCommandCenter::now() const {
    auto tp = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(tp);
    struct tm tm_buf;
    localtime_r(&tt, &tm_buf);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return buf;
}
