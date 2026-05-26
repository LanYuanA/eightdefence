/**
 * @file svc_command_center.hpp
 * @brief 原子服务 - 指挥中心报警（纯软件）
 */

#ifndef SVC_COMMAND_CENTER_HPP
#define SVC_COMMAND_CENTER_HPP

#include "svc_base.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

struct AlertEntry {
    std::string level;    // "high", "medium", "low"
    std::string title;
    std::string message;
    std::string timestamp;
};

class SvcCommandCenter : public SvcBase {
public:
    std::string getName() const override { return "指挥中心报警服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }

    void alert(const std::string& level, const std::string& title, const std::string& message);
    void clearAlerts();
    std::vector<AlertEntry> getAlerts() const;
    std::string getAlertsJson() const;

private:
    std::atomic<bool> active_{false};
    mutable std::mutex mtx_;
    std::vector<AlertEntry> alerts_;

    std::string now() const;
};

#endif /* SVC_COMMAND_CENTER_HPP */
