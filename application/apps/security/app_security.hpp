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
#include "service/atomic/svc_sound_light_alarm.hpp"
#include "service/atomic/svc_drainage.hpp"
#include "service/atomic/svc_temp_humidity_control.hpp"
#include "service/atomic/svc_gas_response.hpp"
#include "service/atomic/svc_command_center.hpp"
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
    // 水浸（模拟值，用于无硬件演示）
    std::atomic<float> waterLevel{0.2f};
    SecurityRiskLevel  waterRisk{SecurityRiskLevel::LOW};

    // 入侵（风险等级，由 evaluateRisk 根据设备数据计算）
    SecurityRiskLevel  intrusionRisk{SecurityRiskLevel::LOW};

    // 气体（云测仪真实传感器 + 模拟覆盖）
    std::atomic<bool>  gasSimulated{false};
    std::atomic<int>   simTvoc{0};
    std::atomic<int>   simCh2o{0};
    std::atomic<int>   simO3{0};
    std::atomic<int>   simCo2{0};
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

    // 服务注入
    void setServices(SvcSoundLightAlarm* a1, SvcDrainage* a2,
                     SvcTempHumidityControl* a3, SvcGasResponse* a4,
                     SvcCommandCenter* a5);

private:
    // API 处理函数
    HttpResponse handleGetStatus(const HttpRequest& req);
    HttpResponse handleGetSensors(const HttpRequest& req);
    HttpResponse handleGetAlarms(const HttpRequest& req);
    HttpResponse handlePostControl(const HttpRequest& req);
    HttpResponse handleGetLogs(const HttpRequest& req);

    // 风险评估（实时从设备读取数据）
    void evaluateRisk(int waterState, int irState, int radarState);
    SecurityRiskLevel calcWaterRisk(float level, int sensorState);
    SecurityRiskLevel calcIntrusionRisk(int irState, int radarState);
    SecurityRiskLevel calcSingleGasRisk(int value, int mediumThreshold, int highThreshold);

    // 日志
    struct LogEntry {
        std::string timestamp;
        std::string level;    // "normal", "warning", "alarm"
        std::string event;
        std::string details;
    };
    void addLog(const std::string& level, const std::string& event, const std::string& details);

    SecurityState m_state;
    std::vector<LogEntry> m_logs;
    mutable std::mutex m_logMutex;

    // 上次风险等级, 用于检测变化避免重复日志
    SecurityRiskLevel m_prevWaterRisk{SecurityRiskLevel::LOW};
    SecurityRiskLevel m_prevIntrusionRisk{SecurityRiskLevel::LOW};
    SecurityRiskLevel m_prevGasRisk{SecurityRiskLevel::LOW};

    // 原子服务指针（由外部注入，不拥有所有权）
    SvcSoundLightAlarm*      m_svcSoundLight = nullptr;
    SvcDrainage*             m_svcDrainage = nullptr;
    SvcTempHumidityControl*  m_svcTempHumid = nullptr;
    SvcGasResponse*          m_svcGasResp = nullptr;
    SvcCommandCenter*        m_svcCmdCenter = nullptr;

    // 风险响应: 根据风险等级驱动原子服务
    void handleRiskResponse();
};

#endif // APP_SECURITY_HPP