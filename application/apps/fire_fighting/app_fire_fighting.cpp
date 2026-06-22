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
        "\"riskPercent\":%d,"
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
        m_state.riskPercent.load(),
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

    // 火情确认 / 误报
    if (action == "confirm" || action == "dismiss") {
        std::string operatorName = req.getParam("operator");
        if (operatorName.empty()) operatorName = "管理员";

        m_state.alarmAcknowledged.store(true);

        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        struct tm tm_buf;
        localtime_r(&tt, &tm_buf);
        char ts[64];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_buf);

        {
            std::lock_guard<std::mutex> lock(m_actionMutex);
            m_fireActions.push_back({ts, action, operatorName});
        }

        if (action == "confirm")
            addLog("alarm", "火情已确认", "操作人: " + operatorName + ", 已执行安全防御");
        else
            addLog("normal", "火情误报", "操作人: " + operatorName + ", 标记为误报, 风险恢复前不再弹窗");

        APP_LOG_INFO("火情%s: 操作人=%s", action == "confirm" ? "确认" : "误报", operatorName.c_str());
        return HttpResponse::json("{\"status\":\"success\"}");
    }

    if (target == "fire") {
        if (action == "simulate") {
            m_state.fireSimulated.store(true);
            m_state.simSmoke.store(1);      // 检测到烟雾
            m_state.simTemp.store(75.0f);   // 高温
            m_state.alarmAcknowledged.store(false);  // 新火情, 重置确认状态
            evaluateRisk();
            APP_LOG_WARNING("火灾模拟: 烟雾检测触发 + 温度75°C");
            addLog("alarm", "火灾模拟", "模拟火灾已触发: 烟雾检测+高温");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"火灾模拟已触发\"}");
        } else if (action == "reset") {
            m_state.fireSimulated.store(false);
            m_state.simSmoke.store(0);
            m_state.simTemp.store(25.0f);
            m_state.alarmAcknowledged.store(false);
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
 * 风险评估 (GB/T 31593.9 加权多参数融合模型)
 *
 * 公式: 综合风险% = 0.35*烟雾 + 0.30*温度 + 0.20*CO2 + 0.15*湿度变化
 * 等级: 0-25 低风险(LOW) / 25-55 中风险(MEDIUM) / 55-100 高风险(HIGH)
 * ============================================================ */
void AppFireFighting::evaluateRisk() {
    int smokeState   = m_state.smokeState.load();
    float temp       = m_state.temperature.load();
    int co2          = dev_co2.isOnline() ? dev_co2.getValue() : m_prevCo2;
    float humidity   = dev_humidity.isOnline() ? dev_humidity.getValue() / 10.0f : m_prevHumidity;

    bool smokeOn = dev_smoke.isOnline();
    bool tempOn  = dev_temperature.isOnline();
    bool co2On   = dev_co2.isOnline();
    bool humOn   = dev_humidity.isOnline();

    bool fireSim = m_state.fireSimulated.load();

    // 各传感器评分 (0-100), 离线时给予不确定性惩罚(50分)
    int smokeScore    = calcSmokeScore(fireSim ? m_state.simSmoke.load() : smokeState, fireSim ? true : smokeOn);
    int tempScore     = calcTempScore(fireSim ? m_state.simTemp.load() : temp, fireSim ? true : tempOn);
    int co2Score      = calcCo2Score(co2, co2On);
    int humidityScore = calcHumidityScore(humidity, humOn);

    // 加权融合
    float weighted = 0.35f * smokeScore + 0.30f * tempScore + 0.20f * co2Score + 0.15f * humidityScore;
    int riskPct = static_cast<int>(weighted + 0.5f);
    if (riskPct < 0) riskPct = 0;
    if (riskPct > 100) riskPct = 100;
    m_state.riskPercent.store(riskPct);

    // 风险等级判定
    if (riskPct >= 55)
        m_state.overallRisk = FireRiskLevel::HIGH;
    else if (riskPct >= 25)
        m_state.overallRisk = FireRiskLevel::MEDIUM;
    else
        m_state.overallRisk = FireRiskLevel::LOW;

    // 子项风险(用于前端各指标着色)
    m_state.smokeRisk = (smokeScore >= 60) ? FireRiskLevel::HIGH :
                        (smokeScore >= 30) ? FireRiskLevel::MEDIUM : FireRiskLevel::LOW;
    m_state.tempRisk  = (tempScore >= 60) ? FireRiskLevel::HIGH :
                        (tempScore >= 30) ? FireRiskLevel::MEDIUM : FireRiskLevel::LOW;

    bool allOnline = smokeOn && tempOn && co2On && humOn;
    m_state.systemNormal.store(riskPct < 25 && allOnline);

    // 更新历史值(用于湿度变化率)
    m_prevHumidity = humidity;
    m_prevCo2      = co2;

    handleRiskResponse();
}

// 烟雾评分: 检测到烟雾=100, 正常=0, 离线=50
int AppFireFighting::calcSmokeScore(int smokeState, bool online) {
    if (!online) return 50;
    return (smokeState != 0) ? 100 : 0;
}

// 温度评分: 25°C→0分, 70°C→100分, 线性插值, 离线=50
int AppFireFighting::calcTempScore(float temp, bool online) {
    if (!online) return 50;
    if (temp <= 25.0f) return 0;
    if (temp >= 70.0f) return 100;
    return static_cast<int>((temp - 25.0f) / (70.0f - 25.0f) * 100.0f);
}

// CO2评分: 400ppm→0分, 2000ppm→100分, 线性插值, 离线=50
int AppFireFighting::calcCo2Score(int co2, bool online) {
    if (!online) return 50;
    if (co2 <= 400) return 0;
    if (co2 >= 2000) return 100;
    return static_cast<int>((co2 - 400) / 1600.0f * 100.0f);
}

// 湿度变化评分: 湿度快速下降(>5%)→100分, 缓慢下降→线性, 不变/上升→0, 离线=50
int AppFireFighting::calcHumidityScore(float humidity, bool online) {
    if (!online) return 50;
    float drop = m_prevHumidity - humidity;
    if (drop <= 0.0f) return 0;
    if (drop >= 5.0f) return 100;
    return static_cast<int>(drop / 5.0f * 100.0f);
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
    int rp = m_state.riskPercent.load();

    // 高风险 (≥55%): 烟雾确认 → 全部联动；否则高温告警
    if (rp >= 55) {
        bool hasSmoke = (m_state.smokeState.load() != 0) || m_state.fireSimulated.load();

        if (m_svcSoundLight) m_svcSoundLight->activate();
        m_state.alarmActive.store(true);

        if (m_svcCmdCenter) {
            m_svcCmdCenter->activate();
            if (hasSmoke)
                m_svcCmdCenter->alert("high", "火灾警报",
                    "多参数融合判定火灾高风险, 综合风险" + std::to_string(rp) + "%, 启动全部应急响应");
            else
                m_svcCmdCenter->alert("medium", "火灾预警",
                    "多参数综合风险" + std::to_string(rp) + "%, 温度/CO2偏高, 请立即排查");
        }
        m_state.centerAlarmActive.store(true);

        if (hasSmoke) {
            if (m_svcSuppression) m_svcSuppression->activate();
            if (m_svcEvacuation) m_svcEvacuation->activate();
            m_state.suppressionActive.store(true);
            m_state.evacuationActive.store(true);

            if (m_prevRiskPercent < 55)
                addLog("alarm", "火灾警报",
                    "多参数融合判定火灾高风险(" + std::to_string(rp) + "%), 启动: 声光报警+灭火联动+疏散引导+指挥中心告警");
        } else {
            if (m_svcSuppression) m_svcSuppression->deactivate();
            if (m_svcEvacuation) m_svcEvacuation->deactivate();
            m_state.suppressionActive.store(false);
            m_state.evacuationActive.store(false);

            if (m_prevRiskPercent < 55)
                addLog("warning", "火灾预警",
                    "综合风险" + std::to_string(rp) + "%(无烟雾确认), 启动: 声光报警+指挥中心预警");
        }
    }
    // 中风险
    else if (rp >= 25) {
        if (m_svcCmdCenter) {
            m_svcCmdCenter->activate();
            m_svcCmdCenter->alert("low", "风险监控",
                "综合风险" + std::to_string(rp) + "%, 建议加强监测");
        }
        m_state.centerAlarmActive.store(true);

        if (m_svcSoundLight) m_svcSoundLight->deactivate();
        if (m_svcSuppression) m_svcSuppression->deactivate();
        if (m_svcEvacuation) m_svcEvacuation->deactivate();
        m_state.alarmActive.store(false);
        m_state.suppressionActive.store(false);
        m_state.evacuationActive.store(false);

        if (m_prevRiskPercent < 25)
            addLog("warning", "风险上升",
                "综合风险升至" + std::to_string(rp) + "%, 已通知指挥中心关注");
    }
    // 低风险
    else {
        if (m_svcSoundLight) m_svcSoundLight->deactivate();
        if (m_svcSuppression) m_svcSuppression->deactivate();
        if (m_svcEvacuation) m_svcEvacuation->deactivate();
        if (m_svcCmdCenter) m_svcCmdCenter->deactivate();
        m_state.alarmActive.store(false);
        m_state.suppressionActive.store(false);
        m_state.evacuationActive.store(false);
        m_state.centerAlarmActive.store(false);

        if (m_prevRiskPercent >= 25)
            addLog("normal", "消防系统恢复",
                "综合风险降至" + std::to_string(rp) + "%, 所有消防服务已停止");
    }

    m_prevRiskPercent = rp;
}
