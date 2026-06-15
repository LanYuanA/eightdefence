/**
 * @file app_environment.hpp
 * @brief 环境监测应用 - 温湿度/PM2.5/PM10/有害气体环境监测
 *
 * 基于八防综合监控网关平台, 提供:
 *   - 温湿度环境监测
 *   - PM2.5/PM10 空气质量监测
 *   - TVOC/CH2O/O3/CO2 有害气体监测
 *   - 环境风险评估与预警
 *   - 原子服务联动（温湿度调控、净化、通风）
 *   - 前端页面服务
 */

#ifndef APP_ENVIRONMENT_HPP
#define APP_ENVIRONMENT_HPP

#include "../../app_base.hpp"
#include "service/atomic/svc_temp_humidity_control.hpp"
#include "service/atomic/svc_gas_response.hpp"
#include "service/atomic/svc_air_quality_alert.hpp"
#include "service/atomic/svc_ventilation.hpp"
#include "service/atomic/svc_command_center.hpp"
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

/**
 * @brief 环境风险等级
 */
enum class EnvRiskLevel {
    LOW = 0,       // 低风险
    MEDIUM = 1,    // 中风险
    HIGH = 2       // 高风险
};

/**
 * @brief 环境监测状态
 */
struct EnvironmentState {
    // 温湿度
    std::atomic<float> temperature{25.0f};
    std::atomic<float> humidity{50.0f};
    EnvRiskLevel       tempRisk{EnvRiskLevel::LOW};
    EnvRiskLevel       humRisk{EnvRiskLevel::LOW};

    // 空气质量
    std::atomic<float> pm25{0.0f};
    std::atomic<float> pm10{0.0f};
    EnvRiskLevel       pmRisk{EnvRiskLevel::LOW};

    // 有害气体
    std::atomic<bool>  gasSimulated{false};
    std::atomic<int>   simTvoc{0};
    std::atomic<int>   simCh2o{0};
    std::atomic<int>   simCo2{0};
    EnvRiskLevel       gasRisk{EnvRiskLevel::LOW};

    // 服务状态
    std::atomic<bool>  tempControlActive{false};
    std::atomic<bool>  gasResponseActive{false};
    std::atomic<bool>  airQualityActive{false};
    std::atomic<bool>  ventilationActive{false};
    std::atomic<bool>  centerAlarmActive{false};

    // 整体状态
    EnvRiskLevel       overallRisk{EnvRiskLevel::LOW};
    std::atomic<bool>  systemNormal{true};
};

class AppEnvironment : public AppBase {
public:
    AppEnvironment();
    ~AppEnvironment() override;

    // 生命周期
    int init() override;
    int start() override;
    void stop() override;

    // API 处理
    HttpResponse handleApi(const HttpRequest& request) override;

    // 服务注入
    void setServices(SvcTempHumidityControl* a1, SvcGasResponse* a2,
                     SvcAirQualityAlert* a3, SvcVentilation* a4,
                     SvcCommandCenter* a5);

private:
    // API 处理函数
    HttpResponse handleGetStatus(const HttpRequest& req);
    HttpResponse handleGetSensors(const HttpRequest& req);
    HttpResponse handlePostControl(const HttpRequest& req);
    HttpResponse handleGetLogs(const HttpRequest& req);

    // 风险评估
    void evaluateRisk();
    EnvRiskLevel calcTempRisk(float temp);
    EnvRiskLevel calcHumRisk(float hum);
    EnvRiskLevel calcPmRisk(float pm25, float pm10);
    EnvRiskLevel calcSingleGasRisk(int value, int mediumThreshold, int highThreshold);

    // 日志
    struct LogEntry {
        std::string timestamp;
        std::string level;
        std::string event;
        std::string details;
    };
    void addLog(const std::string& level, const std::string& event, const std::string& details);

    EnvironmentState m_state;
    std::vector<LogEntry> m_logs;
    mutable std::mutex m_logMutex;

    // 上次风险等级
    EnvRiskLevel m_prevTempRisk{EnvRiskLevel::LOW};
    EnvRiskLevel m_prevHumRisk{EnvRiskLevel::LOW};
    EnvRiskLevel m_prevPmRisk{EnvRiskLevel::LOW};
    EnvRiskLevel m_prevGasRisk{EnvRiskLevel::LOW};

    // 原子服务指针
    SvcTempHumidityControl*  m_svcTempHumid = nullptr;
    SvcGasResponse*          m_svcGasResp = nullptr;
    SvcAirQualityAlert*      m_svcAirQuality = nullptr;
    SvcVentilation*          m_svcVentilation = nullptr;
    SvcCommandCenter*        m_svcCmdCenter = nullptr;

    // 风险响应
    void handleRiskResponse();
};

#endif // APP_ENVIRONMENT_HPP
