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
#include <fstream>
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
    registerApi("/api/fire_actions", [this](const HttpRequest& r){ return handleGetFireActions(r); });
}

AppFireFighting::~AppFireFighting() {
    stop();
}

/* ============================================================
 * 生命周期
 * ============================================================ */
int AppFireFighting::init() {
    loadFireActions();
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
        "\"sprinklerActive\":%s,"
        "\"exhaustActive\":%s,"
        "\"centerAlarmActive\":%s"
        "},"
        "\"devices\":{"
        "\"smoke\":{\"online\":%s},"
        "\"temperature\":{\"online\":%s},"
        "\"humidity\":{\"online\":%s},"
        "\"co2\":{\"online\":%s},"
        "\"alarm\":{\"online\":%s},"
        "\"cabin\":{\"online\":%s,\"running\":%s},"
        "\"sprinkler\":{\"online\":%s,\"running\":%s},"
        "\"exhaustFan\":{\"online\":%s,\"running\":%s}"
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
        m_state.sprinklerActive.load() ? "true" : "false",
        m_state.exhaustActive.load() ? "true" : "false",
        m_state.centerAlarmActive.load() ? "true" : "false",
        smokeOnline ? "true" : "false",
        tempOnline ? "true" : "false",
        humOnline ? "true" : "false",
        co2Online ? "true" : "false",
        alarmOnline ? "true" : "false",
        dev_stepper_cabin.isOnline() ? "true" : "false", dev_stepper_cabin.isRunning() ? "true" : "false",
        dev_stepper_sprinkler.isOnline() ? "true" : "false", dev_stepper_sprinkler.isRunning() ? "true" : "false",
        dev_stepper_exhaust.isOnline() ? "true" : "false", dev_stepper_exhaust.isRunning() ? "true" : "false"
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

    // 设备控制
    if (target == "cabin" || target == "fan" || target == "sprinkler" || target == "horn") {
        bool turnOn = (action == "on");
        if (target == "cabin" && m_svcCabin) {
            int rc = m_svcCabin->control(turnOn);
            if (rc == 0) {
                addLog("normal", turnOn ? "舱门开启" : "舱门关闭", "手动控制");
                return HttpResponse::json("{\"status\":\"success\"}");
            } else {
                addLog("error", turnOn ? "舱门开启失败" : "舱门关闭失败", "rc=" + std::to_string(rc));
                return HttpResponse::error(500, "设备控制失败");
            }
        } else if (target == "fan" && m_svcExhaust) {
            int rc = m_svcExhaust->control(turnOn);
            if (rc == 0) {
                m_state.exhaustActive.store(turnOn);
                addLog("normal", turnOn ? "排烟风机开启" : "排烟风机关闭", "手动控制");
                return HttpResponse::json("{\"status\":\"success\"}");
            } else {
                addLog("error", turnOn ? "排烟风机启动失败" : "排烟风机停止失败", "rc=" + std::to_string(rc));
                return HttpResponse::error(500, "设备控制失败");
            }
        } else if (target == "sprinkler" && m_svcSprinkler) {
            int rc = m_svcSprinkler->control(turnOn);
            if (rc == 0) {
                m_state.sprinklerActive.store(turnOn);
                addLog("normal", turnOn ? "水淋系统开启" : "水淋系统关闭", "手动控制");
                return HttpResponse::json("{\"status\":\"success\"}");
            } else {
                addLog("error", turnOn ? "水淋系统启动失败" : "水淋系统停止失败", "rc=" + std::to_string(rc));
                return HttpResponse::error(500, "设备控制失败");
            }
        } else if (target == "horn") {
            if (turnOn && m_svcSoundLight) m_svcSoundLight->activate();
            else if (!turnOn && m_svcSoundLight) m_svcSoundLight->deactivate();
            addLog("normal", turnOn ? "声光报警器开启" : "声光报警器关闭", "手动控制");
            return HttpResponse::json("{\"status\":\"success\"}");
        }
        addLog("error", target + (turnOn ? "启动失败" : "停止失败"), "服务未初始化");
        return HttpResponse::error(500, "设备控制失败");
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

/* ============================================================
 * 火情操作记录 JSON 持久化
 * ============================================================ */
void AppFireFighting::saveFireActions() {
    std::lock_guard<std::mutex> lock(m_actionMutex);
    system("mkdir -p ./data");
    std::ofstream ofs("./data/fire_actions.json");
    if (!ofs) return;
    ofs << "[\n";
    for (size_t i = 0; i < m_fireActions.size(); i++) {
        if (i > 0) ofs << ",\n";
        ofs << "  {\"timestamp\":\"" << m_fireActions[i].timestamp
            << "\",\"action\":\"" << m_fireActions[i].action
            << "\",\"operator\":\"" << m_fireActions[i].operatorName << "\"}";
    }
    ofs << "\n]\n";
}

void AppFireFighting::loadFireActions() {
    std::lock_guard<std::mutex> lock(m_actionMutex);
    std::ifstream ifs("./data/fire_actions.json");
    if (!ifs) return;
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    // Simple JSON parse: find each { } object
    size_t pos = 0;
    while ((pos = content.find("{\"timestamp\"", pos)) != std::string::npos) {
        size_t end = content.find("}", pos);
        if (end == std::string::npos) break;
        std::string obj = content.substr(pos, end - pos + 1);
        FireActionRecord rec;
        // Extract timestamp
        auto extract = [&](const std::string& key) -> std::string {
            size_t k = obj.find(key);
            if (k == std::string::npos) return "";
            k += key.length() + 3; // skip key":"
            size_t ve = obj.find("\"", k);
            if (ve == std::string::npos) return "";
            return obj.substr(k, ve - k);
        };
        rec.timestamp = extract("timestamp");
        rec.action = extract("action");
        rec.operatorName = extract("operator");
        if (!rec.timestamp.empty())
            m_fireActions.push_back(rec);
        pos = end + 1;
    }
}

HttpResponse AppFireFighting::handleGetFireActions(const HttpRequest& /*req*/) {
    std::lock_guard<std::mutex> lock(m_actionMutex);
    std::string json = "[";
    for (size_t i = 0; i < m_fireActions.size(); i++) {
        if (i > 0) json += ",";
        json += "{\"timestamp\":\"" + m_fireActions[i].timestamp + "\","
                "\"action\":\"" + m_fireActions[i].action + "\","
                "\"operator\":\"" + m_fireActions[i].operatorName + "\"}";
    }
    json += "]";
    return HttpResponse::json(json);
}
void AppFireFighting::evaluateRisk() {
    m_state.smokeRisk = calcSmokeRisk(m_state.smokeState.load());
    m_state.tempRisk  = calcTempRisk(m_state.temperature.load());

    bool smokeOnline = dev_smoke.isOnline();
    bool tempOnline  = dev_temperature.isOnline();

    if (!smokeOnline && !m_state.fireSimulated.load()) m_state.smokeRisk = FireRiskLevel::MEDIUM;
    if (!tempOnline && !m_state.fireSimulated.load())  m_state.tempRisk  = FireRiskLevel::MEDIUM;

    // 加权综合风险计算: 0.35*烟雾 + 0.30*温度 + 0.20*CO2 + 0.15*湿度
    float smokeScore = (m_state.smokeState.load() != 0) ? 100.0f : 0.0f;
    float tempVal = m_state.temperature.load();
    float tempScore = (tempVal > 70.0f) ? 100.0f : (tempVal > 50.0f) ? 60.0f : 20.0f;
    int co2Val = dev_co2.isOnline() ? dev_co2.getValue() : 400;
    float co2Score = (co2Val > 1500) ? 100.0f : (co2Val > 800) ? 60.0f : 20.0f;
    float humVal = dev_humidity.isOnline() ? dev_humidity.getValue() / 10.0f : 60.0f;
    float humidityScore = (humVal < 20.0f || humVal > 90.0f) ? 80.0f : (humVal < 30.0f || humVal > 70.0f) ? 50.0f : 20.0f;

    float weighted = 0.35f * smokeScore + 0.30f * tempScore + 0.20f * co2Score + 0.15f * humidityScore;
    int rp = static_cast<int>(weighted);
    if (rp > 100) rp = 100;
    if (rp < 1) rp = 1;
    m_state.riskPercent.store(rp);

    if (weighted >= 80.0f)
        m_state.overallRisk = FireRiskLevel::HIGH;
    else if (weighted >= 50.0f)
        m_state.overallRisk = FireRiskLevel::MEDIUM;
    else
        m_state.overallRisk = FireRiskLevel::LOW;

    bool allOnline = smokeOnline && tempOnline;
    m_state.systemNormal.store(m_state.overallRisk == FireRiskLevel::LOW && allOnline);

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

void AppFireFighting::setServices(SvcSoundLightAlarm* a1, SvcFireSprinkler* a2, SvcFireFan* a3, SvcCommandCenter* a4, SvcFireCabin* a5) {
    m_svcSoundLight  = a1;
    m_svcSprinkler = a2;
    m_svcExhaust  = a3;
    m_svcCmdCenter   = a4;
    m_svcCabin      = a5;
}

void AppFireFighting::handleRiskResponse() {
    int rp = m_state.riskPercent.load();
    if (rp == m_prevRiskPercent) return;
    m_prevRiskPercent = rp;
    // 烟雾高风险 = 火灾确认
    if (m_state.smokeRisk >= FireRiskLevel::HIGH) {
        // 声光报警
        if (m_svcSoundLight) m_svcSoundLight->activate();
        m_state.alarmActive.store(true);

        // 灭火联动
        if (m_svcSprinkler) m_svcSprinkler->activate();
        m_state.sprinklerActive.store(true);

        // 疏散引导
        if (m_svcExhaust) m_svcExhaust->activate();
        m_state.exhaustActive.store(true);

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
        if (m_svcSprinkler) m_svcSprinkler->deactivate();
        if (m_svcExhaust) m_svcExhaust->deactivate();
        m_state.sprinklerActive.store(false);
        m_state.exhaustActive.store(false);

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
        if (m_svcSprinkler) m_svcSprinkler->deactivate();
        if (m_svcExhaust) m_svcExhaust->deactivate();
        m_state.alarmActive.store(false);
        m_state.sprinklerActive.store(false);
        m_state.exhaustActive.store(false);

        if (m_prevTempRisk < FireRiskLevel::MEDIUM) {
            addLog("warning", "温度偏高", "环境温度超过50°C, 已通知指挥中心");
        }
    }
    // 安全状态
    else {
        if (m_svcSoundLight) m_svcSoundLight->deactivate();
        if (m_svcSprinkler) m_svcSprinkler->deactivate();
        if (m_svcExhaust) m_svcExhaust->deactivate();
        if (m_svcCmdCenter) m_svcCmdCenter->deactivate();
        m_state.alarmActive.store(false);
        m_state.sprinklerActive.store(false);
        m_state.exhaustActive.store(false);
        m_state.centerAlarmActive.store(false);

        if (m_prevSmokeRisk >= FireRiskLevel::HIGH || m_prevTempRisk >= FireRiskLevel::MEDIUM) {
            addLog("normal", "消防系统恢复", "所有风险已解除, 消防服务已停止");
        }
    }

    m_prevSmokeRisk = m_state.smokeRisk;
    m_prevTempRisk  = m_state.tempRisk;
}
