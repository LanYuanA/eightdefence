/**
 * @file app_fire_fighting.hpp
 * @brief 消防系统应用 - 烟雾/温度/CO2/湿度多参数火灾风险检测与应急处置
 *
 * 基于八防综合监控网关平台和 GB/T 31593.9 火灾风险评估框架
 *
 * 风险评估模型：
 *   综合风险% = 0.35×烟雾得分 + 0.30×温度得分 + 0.20×CO2得分 + 0.15×湿度变化得分
 *   等级划分：0-25低风险 / 25-55中风险 / 55-100高风险
 */

#ifndef APP_FIRE_FIGHTING_HPP
#define APP_FIRE_FIGHTING_HPP

#include "../../app_base.hpp"
#include "service/atomic/svc_sound_light_alarm.hpp"
#include "service/atomic/svc_fire_suppression.hpp"
#include "service/atomic/svc_evacuation.hpp"
#include "service/atomic/svc_command_center.hpp"
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

enum class FireRiskLevel {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
};

struct FireFightingState {
    std::atomic<int>   smokeState{0};
    FireRiskLevel      smokeRisk{FireRiskLevel::LOW};
    std::atomic<float> temperature{25.0f};
    FireRiskLevel      tempRisk{FireRiskLevel::LOW};
    std::atomic<bool>  fireSimulated{false};
    std::atomic<int>   simSmoke{0};
    std::atomic<float> simTemp{25.0f};
    std::atomic<bool>  alarmActive{false};
    std::atomic<bool>  suppressionActive{false};
    std::atomic<bool>  evacuationActive{false};
    std::atomic<bool>  centerAlarmActive{false};
    FireRiskLevel      overallRisk{FireRiskLevel::LOW};
    std::atomic<int>   riskPercent{15};
    std::atomic<bool>  systemNormal{true};
    std::atomic<bool>  alarmAcknowledged{false};
};

class AppFireFighting : public AppBase {
public:
    AppFireFighting();
    ~AppFireFighting() override;

    int init() override;
    int start() override;
    void stop() override;
    HttpResponse handleApi(const HttpRequest& request) override;
    void setServices(SvcSoundLightAlarm* a1, SvcFireSuppression* a2,
                     SvcEvacuation* a3, SvcCommandCenter* a4);

private:
    HttpResponse handleGetStatus(const HttpRequest& req);
    HttpResponse handleGetSensors(const HttpRequest& req);
    HttpResponse handlePostControl(const HttpRequest& req);
    HttpResponse handleGetLogs(const HttpRequest& req);

    void evaluateRisk();
    int calcSmokeScore(int smokeState, bool online);
    int calcTempScore(float temp, bool online);
    int calcCo2Score(int co2, bool online);
    int calcHumidityScore(float humidity, bool online);

    struct LogEntry {
        std::string timestamp;
        std::string level;
        std::string event;
        std::string details;
    };
    void addLog(const std::string& level, const std::string& event, const std::string& details);

    FireFightingState m_state;
    std::vector<LogEntry> m_logs;
    mutable std::mutex m_logMutex;

    // 火情确认/误报操作记录
    struct FireActionRecord {
        std::string timestamp;
        std::string action;
        std::string operatorName;
    };
    std::vector<FireActionRecord> m_fireActions;
    mutable std::mutex m_actionMutex;

    float m_prevHumidity{60.0f};
    int   m_prevCo2{400};
    int   m_prevRiskPercent{15};

    SvcSoundLightAlarm*    m_svcSoundLight = nullptr;
    SvcFireSuppression*    m_svcSuppression = nullptr;
    SvcEvacuation*         m_svcEvacuation = nullptr;
    SvcCommandCenter*      m_svcCmdCenter = nullptr;

    void handleRiskResponse();
};

#endif
