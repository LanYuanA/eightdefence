/**
 * @file app_fire_fighting.cpp
 * @brief 消防系统应用实现
 */

#include "app_fire_fighting.hpp"
#include "../../../core/global_devices.hpp"
#include "../../../core/logger.hpp"
#include "../../../core/app_logger.hpp"

#include <cstdio>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

/* ============================================================
 * 构造 / 析构
 * ============================================================ */
AppFireFighting::AppFireFighting()
    : AppBase("fire_fighting", "/fire", "智能消防系统 - 烟雾/温度火灾检测与应急处置")
{
    registerApi("/api/status",  [this](const HttpRequest& r){ return handleGetStatus(r); });
    registerApi("/api/sensors", [this](const HttpRequest& r){ return handleGetSensors(r); });
    registerApi("/api/control", [this](const HttpRequest& r){ return handlePostControl(r); });
    registerApi("/api/logs",    [this](const HttpRequest& r){ return handleGetLogs(r); });
}

AppFireFighting::~AppFireFighting() {
    stop();
}

/* ============================================================
 * 生命周期
 * ============================================================ */
int AppFireFighting::init() {
    APP_LOG_INFO("初始化...");
    addLog("normal", "系统初始化", "消防应用正在初始化, 加载传感器配置");
    return 0;
}

int AppFireFighting::start() {
    AppBase::start();
    APP_LOG_INFO("启动, 路由前缀: %s", m_routePrefix.c_str());
    addLog("normal", "系统启动完成", "烟雾探测器和温度传感器初始化成功, 系统进入正常运行状态");
    return 0;
}

void AppFireFighting::stop() {
    if (m_running.load()) {
        AppBase::stop();
        APP_LOG_INFO("已停止");
    }
}

/* ============================================================
 * API 处理
 * ============================================================ */
HttpResponse AppFireFighting::handleApi(const HttpRequest& request) {
    for (const auto& route : m_apiRoutes) {
        if (request.path == route.first) {
            return route.second(request);
        }
    }
    return HttpResponse::error(404, "API not found: " + request.path);
}

HttpResponse AppFireFighting::handleGetStatus(const HttpRequest& /*req*/) {
    // 实时从设备读取数据
    int smokeState;
    float temp;

    if (m_state.fireSimulated.load()) {
        smokeState = m_state.simSmoke.load();
        temp = m_state.simTemp.load();
    } else {
        smokeState = dev_smoke.isOnline() ? dev_smoke.getAlarmState() : 0;
        temp = dev_temperature.isOnline() ? dev_temperature.getValue() / 10.0f : 25.0f;
    }

    m_state.smokeState.store(smokeState);
    m_state.temperature.store(temp);

    // 从云测仪读取湿度和CO2
    float hum = dev_humidity.isOnline() ? dev_humidity.getValue() / 10.0f : 0.0f;
    int co2 = dev_co2.isOnline() ? dev_co2.getValue() : 0;

    // 设备在线状态
    bool smokeOnline = dev_smoke.isOnline();
    bool tempOnline  = dev_temperature.isOnline();
    bool humOnline   = dev_humidity.isOnline();
    bool co2Online   = dev_co2.isOnline();
    bool alarmOnline = dev_alarm.isOnline();

    // 实时计算风险等级
    evaluateRisk();

    char json[2048];
    const char* riskStr[] = {"安全", "预警", "火灾"};

    snprintf(json, sizeof(json),
        "{"
        "\"system\":{"
        "\"overallRisk\":\"%s\","
        "\"systemNormal\":%s,"
        "\"running\":%s,"
        "\"fireSimulated\":%s"
        "},"
        "\"smoke\":{"
        "\"state\":%d,"
        "\"stateText\":\"%s\","
        "\"risk\":\"%s\","
        "\"online\":%s"
        "},"
        "\"temperature\":{"
        "\"value\":%.1f,"
        "\"unit\":\"°C\","
        "\"risk\":\"%s\","
        "\"online\":%s"
        "},"
        "\"humidity\":{"
        "\"value\":%.1f,"
        "\"unit\":\"%%\","
        "\"online\":%s"
        "},"
        "\"co2\":{"
        "\"value\":%d,"
        "\"unit\":\"ppm\","
        "\"online\":%s"
        "},"
        "\"services\":{"
        "\"alarmActive\":%s,"
        "\"suppressionActive\":%s,"
        "\"evacuationActive\":%s,"
        "\"centerAlarmActive\":%s"
        "},"
        "\"devices\":{"
        "\"smoke\":{\"online\":%s},"
        "\"temperature\":{\"online\":%s},"
        "\"humidity\":{\"online\":%s},"
        "\"co2\":{\"online\":%s},"
        "\"alarm\":{\"online\":%s}"
        "}"
        "}",
        riskStr[static_cast<int>(m_state.overallRisk)],
        m_state.systemNormal.load() ? "true" : "false",
        m_running.load() ? "true" : "false",
        m_state.fireSimulated.load() ? "true" : "false",
        smokeState,
        smokeState == 0 ? "正常" : "检测到烟雾",
        riskStr[static_cast<int>(m_state.smokeRisk)],
        smokeOnline ? "true" : "false",
        temp,
        riskStr[static_cast<int>(m_state.tempRisk)],
        tempOnline ? "true" : "false",
        hum,
        humOnline ? "true" : "false",
        co2,
        co2Online ? "true" : "false",
        m_state.alarmActive.load() ? "true" : "false",
        m_state.suppressionActive.load() ? "true" : "false",
        m_state.evacuationActive.load() ? "true" : "false",
        m_state.centerAlarmActive.load() ? "true" : "false",
        smokeOnline ? "true" : "false",
        tempOnline ? "true" : "false",
        humOnline ? "true" : "false",
        co2Online ? "true" : "false",
        alarmOnline ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppFireFighting::handleGetSensors(const HttpRequest& /*req*/) {
    int smokeState;
    float temp;

    if (m_state.fireSimulated.load()) {
        smokeState = m_state.simSmoke.load();
        temp = m_state.simTemp.load();
    } else {
        smokeState = dev_smoke.isOnline() ? dev_smoke.getAlarmState() : 0;
        temp = dev_temperature.isOnline() ? dev_temperature.getValue() / 10.0f : 0.0f;
    }

    char json[1024];
    snprintf(json, sizeof(json),
        "["
        "{\"name\":\"烟雾探测器\",\"type\":\"smoke\",\"state\":%d,\"stateText\":\"%s\",\"online\":%s},"
        "{\"name\":\"温度传感器\",\"type\":\"temperature\",\"value\":%.1f,\"unit\":\"°C\",\"online\":%s}"
        "]",
        smokeState, smokeState == 0 ? "正常" : "检测到烟雾", dev_smoke.isOnline() ? "true" : "false",
        temp, dev_temperature.isOnline() ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppFireFighting::handlePostControl(const HttpRequest& req) {
    std::string action = req.getParam("action");
    std::string target = req.getParam("target");

    if (target == "fire") {
        if (action == "simulate") {
            m_state.fireSimulated.store(true);
            m_state.simSmoke.store(1);      // 检测到烟雾
            m_state.simTemp.store(75.0f);   // 高温
            evaluateRisk();
            APP_LOG_WARNING("火灾模拟: 烟雾检测触发 + 温度75°C");
            addLog("alarm", "火灾模拟", "模拟火灾已触发: 烟雾检测+高温");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"火灾模拟已触发\"}");
        } else if (action == "reset") {
            m_state.fireSimulated.store(false);
            m_state.simSmoke.store(0);
            m_state.simTemp.store(25.0f);
            evaluateRisk();
            APP_LOG_INFO("火灾模拟取消: 恢复正常");
            addLog("normal", "火灾警报解除", "火灾模拟已取消, 恢复正常状态");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"火灾警报已解除\"}");
        }
    } else if (target == "smoke") {
        if (action == "simulate") {
            m_state.fireSimulated.store(true);
            m_state.simSmoke.store(1);
            m_state.simTemp.store(30.0f);
            evaluateRisk();
            APP_LOG_WARNING("烟雾模拟: 检测到烟雾");
            addLog("alarm", "烟雾模拟", "模拟烟雾检测已触发");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"烟雾模拟已触发\"}");
        } else if (action == "reset") {
            m_state.fireSimulated.store(false);
            m_state.simSmoke.store(0);
            m_state.simTemp.store(25.0f);
            evaluateRisk();
            APP_LOG_INFO("烟雾模拟取消");
            addLog("normal", "烟雾警报解除", "烟雾模拟已取消");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"烟雾警报已解除\"}");
        }
    }

    return HttpResponse::error(400, "无效的控制指令");
}

HttpResponse AppFireFighting::handleGetLogs(const HttpRequest& req) {
    int limit = 50;
    std::string limitStr = req.getParam("limit");
    if (!limitStr.empty()) {
        limit = std::stoi(limitStr);
    }

    std::lock_guard<std::mutex> lock(m_logMutex);
    std::string json = "[";
    int count = 0;
    for (int i = static_cast<int>(m_logs.size()) - 1; i >= 0 && count < limit; i--, count++) {
        if (count > 0) json += ",";
        json += "{\"timestamp\":\"" + m_logs[i].timestamp + "\","
                "\"level\":\"" + m_logs[i].level + "\","
                "\"event\":\"" + m_logs[i].event + "\","
                "\"details\":\"" + m_logs[i].details + "\"}";
    }
    json += "]";
    return HttpResponse::json(json);
}

/* ============================================================
 * 业务逻辑
 * ============================================================ */
void AppFireFighting::evaluateRisk() {
    m_state.smokeRisk = calcSmokeRisk(m_state.smokeState.load());
    m_state.tempRisk  = calcTempRisk(m_state.temperature.load());

    bool smokeOnline = dev_smoke.isOnline();
    bool tempOnline  = dev_temperature.isOnline();

    // 设备离线时标记中风险
    if (!smokeOnline && !m_state.fireSimulated.load()) m_state.smokeRisk = FireRiskLevel::MEDIUM;
    if (!tempOnline && !m_state.fireSimulated.load())  m_state.tempRisk  = FireRiskLevel::MEDIUM;

    // 整体风险取最高
    int maxRisk = std::max({
        static_cast<int>(m_state.smokeRisk),
        static_cast<int>(m_state.tempRisk)
    });
    m_state.overallRisk = static_cast<FireRiskLevel>(maxRisk);

    bool allOnline = smokeOnline && tempOnline;
    m_state.systemNormal.store(maxRisk == 0 && allOnline);

    handleRiskResponse();
}

FireRiskLevel AppFireFighting::calcSmokeRisk(int smokeState) {
    if (smokeState != 0) return FireRiskLevel::HIGH;
    return FireRiskLevel::LOW;
}

FireRiskLevel AppFireFighting::calcTempRisk(float temp) {
    if (temp > 70.0f) return FireRiskLevel::HIGH;
    if (temp > 50.0f) return FireRiskLevel::MEDIUM;
    return FireRiskLevel::LOW;
}

void AppFireFighting::addLog(const std::string& level, const std::string& event, const std::string& details) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
    localtime_r(&time_t_now, &tm_buf);

    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &tm_buf);

    LogEntry entry;
    entry.timestamp = timeStr;
    entry.level = level;
    entry.event = event;
    entry.details = details;

    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logs.push_back(std::move(entry));

    if (m_logs.size() > 500) {
        m_logs.erase(m_logs.begin(), m_logs.begin() + 100);
    }
}

void AppFireFighting::setServices(SvcSoundLightAlarm* a1, SvcFireSuppression* a2,
                                   SvcEvacuation* a3, SvcCommandCenter* a4) {
    m_svcSoundLight  = a1;
    m_svcSuppression = a2;
    m_svcEvacuation  = a3;
    m_svcCmdCenter   = a4;
}

void AppFireFighting::handleRiskResponse() {
    // 烟雾高风险 = 火灾确认
    if (m_state.smokeRisk >= FireRiskLevel::HIGH) {
        // 声光报警
        if (m_svcSoundLight) m_svcSoundLight->activate();
        m_state.alarmActive.store(true);

        // 灭火联动
        if (m_svcSuppression) m_svcSuppression->activate();
        m_state.suppressionActive.store(true);

        // 疏散引导
        if (m_svcEvacuation) m_svcEvacuation->activate();
        m_state.evacuationActive.store(true);

        // 指挥中心告警
        if (m_svcCmdCenter) {
            m_svcCmdCenter->activate();
            m_svcCmdCenter->alert("high", "火灾警报",
                "烟雾探测器检测到烟雾, 火灾风险等级: 高风险, 灭火和疏散系统已启动");
        }
        m_state.centerAlarmActive.store(true);

        if (m_prevSmokeRisk < FireRiskLevel::HIGH) {
            addLog("alarm", "火灾警报",
                "烟雾探测器检测到烟雾, 触发火灾应急响应: 声光报警+灭火联动+疏散引导+指挥中心告警");
        }
    }
    // 温度高风险但无烟雾 = 预警
    else if (m_state.tempRisk >= FireRiskLevel::HIGH) {
        // 声光报警
        if (m_svcSoundLight) m_svcSoundLight->activate();
        m_state.alarmActive.store(true);

        // 指挥中心预警
        if (m_svcCmdCenter) {
            m_svcCmdCenter->activate();
            m_svcCmdCenter->alert("medium", "高温预警",
                "环境温度超过70°C, 存在火灾风险, 请立即排查");
        }
        m_state.centerAlarmActive.store(true);

        // 灭火和疏散暂不启动（仅温度高，无烟雾确认）
        if (m_svcSuppression) m_svcSuppression->deactivate();
        if (m_svcEvacuation) m_svcEvacuation->deactivate();
        m_state.suppressionActive.store(false);
        m_state.evacuationActive.store(false);

        if (m_prevTempRisk < FireRiskLevel::HIGH) {
            addLog("warning", "高温预警",
                "环境温度超过70°C, 触发高温预警: 声光报警+指挥中心预警");
        }
    }
    // 温度中风险 = 预警
    else if (m_state.tempRisk >= FireRiskLevel::MEDIUM) {
        if (m_svcCmdCenter) {
            m_svcCmdCenter->activate();
            m_svcCmdCenter->alert("low", "温度偏高",
                "环境温度超过50°C, 请注意监控");
        }
        m_state.centerAlarmActive.store(true);

        if (m_svcSoundLight) m_svcSoundLight->deactivate();
        if (m_svcSuppression) m_svcSuppression->deactivate();
        if (m_svcEvacuation) m_svcEvacuation->deactivate();
        m_state.alarmActive.store(false);
        m_state.suppressionActive.store(false);
        m_state.evacuationActive.store(false);

        if (m_prevTempRisk < FireRiskLevel::MEDIUM) {
            addLog("warning", "温度偏高", "环境温度超过50°C, 已通知指挥中心");
        }
    }
    // 安全状态
    else {
        if (m_svcSoundLight) m_svcSoundLight->deactivate();
        if (m_svcSuppression) m_svcSuppression->deactivate();
        if (m_svcEvacuation) m_svcEvacuation->deactivate();
        if (m_svcCmdCenter) m_svcCmdCenter->deactivate();
        m_state.alarmActive.store(false);
        m_state.suppressionActive.store(false);
        m_state.evacuationActive.store(false);
        m_state.centerAlarmActive.store(false);

        if (m_prevSmokeRisk >= FireRiskLevel::HIGH || m_prevTempRisk >= FireRiskLevel::MEDIUM) {
            addLog("normal", "消防系统恢复", "所有风险已解除, 消防服务已停止");
        }
    }

    m_prevSmokeRisk = m_state.smokeRisk;
    m_prevTempRisk  = m_state.tempRisk;
}
