/**
 * @file app_security.hpp
 * @brief 安防系统应用 - 水浸/入侵/气体检测安防场景
 *
 * 基于 temp/安保.html 参考设计, 提供:
 *   - 水浸风险安防监测
 *   - 人员入侵检测
 *   - 有害气体安全防护
 *   - 报警联动服务
 *   - 前端页面服务
 */

#ifndef APP_SECURITY_HPP
#define APP_SECURITY_HPP

#include "../../app_base.hpp"
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

/**
 * @brief 安防系统风险等级
 */
enum class SecurityRiskLevel {
    LOW = 0,       // 低风险
    MEDIUM = 1,    // 中风险
    HIGH = 2       // 高风险
};

/**
 * @brief 安防系统状态
 */
struct SecurityState {
    // 水浸
    std::atomic<float> waterLevel{0.2f};
    std::atomic<int>   waterSensorState{0};   // 0=正常
    SecurityRiskLevel  waterRisk{SecurityRiskLevel::LOW};

    // 入侵
    std::atomic<int>   infraredState{0};      // 0=无入侵
    std::atomic<int>   radarState{0};         // 雷达状态
    SecurityRiskLevel  intrusionRisk{SecurityRiskLevel::LOW};

    // 气体
    std::atomic<int>   gasConcentration{12};  // PPM
    SecurityRiskLevel  gasRisk{SecurityRiskLevel::LOW};

    // 报警服务
    std::atomic<bool>  alarmSoundActive{false};
    std::atomic<bool>  alarmCenterActive{false};

    // 防护处置
    std::atomic<bool>  ventilationActive{false};
    std::atomic<bool>  waterControlActive{false};

    // 整体状态
    SecurityRiskLevel  overallRisk{SecurityRiskLevel::LOW};
    std::atomic<bool>  systemNormal{true};
};

class AppSecurity : public AppBase {
public:
    AppSecurity();
    ~AppSecurity() override;

    // 生命周期
    int init() override;
    int start() override;
    void stop() override;

    // 前端
    std::string getStaticDir() const override;
    std::string getIndexPage() const override;

    // API 处理
    HttpResponse handleApi(const HttpRequest& request) override;

    // 业务方法
    const SecurityState& getState() const { return m_state; }
    void triggerAlarm(const std::string& type, const std::string& message);
    void clearAlarm(const std::string& type);

private:
    // API 处理函数
    HttpResponse handleGetStatus(const HttpRequest& req);
    HttpResponse handleGetSensors(const HttpRequest& req);
    HttpResponse handleGetAlarms(const HttpRequest& req);
    HttpResponse handlePostControl(const HttpRequest& req);
    HttpResponse handleGetLogs(const HttpRequest& req);

    // 业务线程
    void pollingThread();

    // 风险评估
    void evaluateRisk();
    void updateWaterRisk();
    void updateIntrusionRisk();
    void updateGasRisk();

    // 日志
    struct LogEntry {
        std::string timestamp;
        std::string level;    // "normal", "warning", "alarm"
        std::string event;
        std::string details;
    };
    void addLog(const std::string& level, const std::string& event, const std::string& details);

    SecurityState m_state;
    std::thread   m_pollThread;
    std::vector<LogEntry> m_logs;
    mutable std::mutex m_logMutex;

    // 模拟模式 (用于无硬件环境的演示)
    bool m_simulationMode = false;
};

#endif // APP_SECURITY_HPP