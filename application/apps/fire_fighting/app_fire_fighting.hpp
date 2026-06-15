/**
 * @file app_fire_fighting.hpp
 * @brief 消防系统应用 - 烟雾/温度火灾检测与应急处置
 *
 * 基于八防综合监控网关平台, 提供:
 *   - 烟雾浓度监测
 *   - 温度火灾风险评估
 *   - 火灾等级综合判定
 *   - 灭火联动与疏散引导
 *   - 前端页面服务
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

/**
 * @brief 火灾风险等级
 */
enum class FireRiskLevel {
    LOW = 0,       // 低风险 (安全)
    MEDIUM = 1,    // 中风险 (预警)
    HIGH = 2       // 高风险 (火灾)
};

/**
 * @brief 消防系统状态
 */
struct FireFightingState {
    // 烟雾
    std::atomic<int>   smokeState{0};       // 0=正常, 非0=检测到烟雾
    FireRiskLevel      smokeRisk{FireRiskLevel::LOW};

    // 温度
    std::atomic<float> temperature{25.0f};
    FireRiskLevel      tempRisk{FireRiskLevel::LOW};

    // 火灾模拟
    std::atomic<bool>  fireSimulated{false};
    std::atomic<int>   simSmoke{0};
    std::atomic<float> simTemp{25.0f};

    // 服务状态
    std::atomic<bool>  alarmActive{false};
    std::atomic<bool>  suppressionActive{false};
    std::atomic<bool>  evacuationActive{false};
    std::atomic<bool>  centerAlarmActive{false};

    // 整体状态
    FireRiskLevel      overallRisk{FireRiskLevel::LOW};
    std::atomic<bool>  systemNormal{true};
};

class AppFireFighting : public AppBase {
public:
    AppFireFighting();
    ~AppFireFighting() override;

    // 生命周期
    int init() override;
    int start() override;
    void stop() override;

    // API 处理
    HttpResponse handleApi(const HttpRequest& request) override;

    // 服务注入
    void setServices(SvcSoundLightAlarm* a1, SvcFireSuppression* a2,
                     SvcEvacuation* a3, SvcCommandCenter* a4);

private:
    // API 处理函数
    HttpResponse handleGetStatus(const HttpRequest& req);
    HttpResponse handleGetSensors(const HttpRequest& req);
    HttpResponse handlePostControl(const HttpRequest& req);
    HttpResponse handleGetLogs(const HttpRequest& req);

    // 风险评估
    void evaluateRisk();
    FireRiskLevel calcSmokeRisk(int smokeState);
    FireRiskLevel calcTempRisk(float temp);

    // 日志
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

    // 上次风险等级
    FireRiskLevel m_prevSmokeRisk{FireRiskLevel::LOW};
    FireRiskLevel m_prevTempRisk{FireRiskLevel::LOW};

    // 原子服务指针
    SvcSoundLightAlarm*    m_svcSoundLight = nullptr;
    SvcFireSuppression*    m_svcSuppression = nullptr;
    SvcEvacuation*         m_svcEvacuation = nullptr;
    SvcCommandCenter*      m_svcCmdCenter = nullptr;

    // 风险响应
    void handleRiskResponse();
};

#endif // APP_FIRE_FIGHTING_HPP
