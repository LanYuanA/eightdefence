/**
 * @file app_security.cpp
 * @brief 安防系统应用实现
 */

#include "app_security.hpp"
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
AppSecurity::AppSecurity()
    : AppBase("security", "/security", "智能安防系统 - 水浸/入侵/气体检测安防场景")
{
    // 注册 API 路由
    registerApi("/api/status",  [this](const HttpRequest& r){ return handleGetStatus(r); });
    registerApi("/api/sensors", [this](const HttpRequest& r){ return handleGetSensors(r); });
    registerApi("/api/alarms",  [this](const HttpRequest& r){ return handleGetAlarms(r); });
    registerApi("/api/control", [this](const HttpRequest& r){ return handlePostControl(r); });
    registerApi("/api/logs",    [this](const HttpRequest& r){ return handleGetLogs(r); });
}

AppSecurity::~AppSecurity() {
    stop();
}

/* ============================================================
 * 生命周期
 * ============================================================ */
int AppSecurity::init() {
    APP_LOG_INFO("初始化...");
    addLog("normal", "系统初始化", "安防应用正在初始化, 加载传感器配置");
    return 0;
}

int AppSecurity::start() {
    AppBase::start();
    APP_LOG_INFO("启动, 路由前缀: %s", m_routePrefix.c_str());
    addLog("normal", "系统启动完成", "所有安防传感器初始化成功, 系统进入正常运行状态");
    return 0;
}

void AppSecurity::stop() {
    if (m_running.load()) {
        AppBase::stop();
        APP_LOG_INFO("已停止");
    }
}

/* ============================================================
 * 前端文件
 * ============================================================ */
std::string AppSecurity::getStaticDir() const {
    return "public/apps/security";
}

std::string AppSecurity::getIndexPage() const {
    return "public/apps/security/index.html";
}

/* ============================================================
 * API 处理
 * ============================================================ */
HttpResponse AppSecurity::handleApi(const HttpRequest& request) {
    // 路由已经在构造函数中注册, 直接使用基类的路由匹配
    for (const auto& route : m_apiRoutes) {
        if (request.path == route.first) {
            return route.second(request);
        }
    }
    return HttpResponse::error(404, "API not found: " + request.path);
}

HttpResponse AppSecurity::handleGetStatus(const HttpRequest& /*req*/) {
    // 实时从设备读取数据
    int waterState = dev_water.getWaterState();
    int irState = dev_infrared.getInfraredState();
    int radarState = dev_infrared.getRadarState();
    float waterLevel = m_state.waterLevel.load();

    // 设备在线状态
    bool waterOnline = dev_water.isOnline();
    bool irOnline = dev_infrared.isOnline();

    // 读取云测仪有害气体传感器
    bool tvocOnline = dev_tvoc.isOnline();
    bool ch2oOnline = dev_ch2o.isOnline();
    bool o3Online   = dev_o3.isOnline();
    bool co2Online  = dev_co2.isOnline();
    bool gasOnline  = tvocOnline || ch2oOnline || o3Online || co2Online;

    int tvocVal, ch2oVal, o3Val, co2Val;
    if (m_state.gasSimulated.load()) {
        tvocVal = m_state.simTvoc.load();
        ch2oVal = m_state.simCh2o.load();
        o3Val   = m_state.simO3.load();
        co2Val  = m_state.simCo2.load();
    } else {
        tvocVal = dev_tvoc.getValue();
        ch2oVal = dev_ch2o.getValue();
        o3Val   = dev_o3.getValue();
        co2Val  = dev_co2.getValue();
    }

    // 实时计算风险等级
    evaluateRisk(waterState, irState, radarState);

    char json[4096];
    const char* riskStr[] = {"低风险", "中风险", "高风险"};

    snprintf(json, sizeof(json),
        "{"
        "\"system\":{"
        "\"overallRisk\":\"%s\","
        "\"systemNormal\":%s,"
        "\"running\":%s"
        "},"
        "\"water\":{"
        "\"level\":%.1f,"
        "\"sensorState\":%d,"
        "\"risk\":\"%s\","
        "\"controlActive\":%s,"
        "\"online\":%s"
        "},"
        "\"intrusion\":{"
        "\"infraredState\":%d,"
        "\"radarState\":%d,"
        "\"risk\":\"%s\","
        "\"online\":%s"
        "},"
        "\"gas\":{"
        "\"online\":%s,"
        "\"tvoc\":{\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "\"ch2o\":{\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "\"o3\":{\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "\"co2\":{\"value\":%d,\"unit\":\"ppm\",\"online\":%s},"
        "\"risk\":\"%s\","
        "\"ventilationActive\":%s"
        "},"
        "\"alarm\":{"
        "\"soundActive\":%s,"
        "\"centerActive\":%s"
        "}"
        "}",
        riskStr[static_cast<int>(m_state.overallRisk)],
        m_state.systemNormal.load() ? "true" : "false",
        m_running.load() ? "true" : "false",
        waterLevel,
        waterState,
        riskStr[static_cast<int>(m_state.waterRisk)],
        m_state.waterControlActive.load() ? "true" : "false",
        waterOnline ? "true" : "false",
        irState,
        radarState,
        riskStr[static_cast<int>(m_state.intrusionRisk)],
        irOnline ? "true" : "false",
        gasOnline ? "true" : "false",
        tvocVal, tvocOnline ? "true" : "false",
        ch2oVal, ch2oOnline ? "true" : "false",
        o3Val,   o3Online   ? "true" : "false",
        co2Val,  co2Online  ? "true" : "false",
        riskStr[static_cast<int>(m_state.gasRisk)],
        m_state.ventilationActive.load() ? "true" : "false",
        m_state.alarmSoundActive.load() ? "true" : "false",
        m_state.alarmCenterActive.load() ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppSecurity::handleGetSensors(const HttpRequest& /*req*/) {
    int waterState = dev_water.getWaterState();
    int irState = dev_infrared.getInfraredState();
    int radarState = dev_infrared.getRadarState();
    bool waterOnline = dev_water.isOnline();
    bool irOnline = dev_infrared.isOnline();

    char json[2048];
    snprintf(json, sizeof(json),
        "["
        "{\"name\":\"水浸传感器\",\"type\":\"water\",\"state\":%d,\"value\":%.1f,\"unit\":\"cm\",\"online\":%s},"
        "{\"name\":\"红外探测器\",\"type\":\"infrared\",\"state\":%d,\"value\":%d,\"unit\":\"\",\"online\":%s},"
        "{\"name\":\"雷达探测器\",\"type\":\"radar\",\"state\":%d,\"value\":%d,\"unit\":\"\",\"online\":%s},"
        "{\"name\":\"TVOC传感器\",\"type\":\"tvoc\",\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "{\"name\":\"甲醛传感器\",\"type\":\"ch2o\",\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "{\"name\":\"臭氧传感器\",\"type\":\"o3\",\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "{\"name\":\"CO2传感器\",\"type\":\"co2\",\"value\":%d,\"unit\":\"ppm\",\"online\":%s}"
        "]",
        waterState, m_state.waterLevel.load(), waterOnline ? "true" : "false",
        irState, irState, irOnline ? "true" : "false",
        radarState, radarState, irOnline ? "true" : "false",
        dev_tvoc.getValue(), dev_tvoc.isOnline() ? "true" : "false",
        dev_ch2o.getValue(), dev_ch2o.isOnline() ? "true" : "false",
        dev_o3.getValue(),   dev_o3.isOnline()   ? "true" : "false",
        dev_co2.getValue(),  dev_co2.isOnline()  ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppSecurity::handleGetAlarms(const HttpRequest& /*req*/) {
    char json[512];
    snprintf(json, sizeof(json),
        "{"
        "\"soundAlarm\":%s,"
        "\"centerAlarm\":%s,"
        "\"ventilation\":%s,"
        "\"waterControl\":%s"
        "}",
        m_state.alarmSoundActive.load() ? "true" : "false",
        m_state.alarmCenterActive.load() ? "true" : "false",
        m_state.ventilationActive.load() ? "true" : "false",
        m_state.waterControlActive.load() ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppSecurity::handlePostControl(const HttpRequest& req) {
    std::string action = req.getParam("action");
    std::string target = req.getParam("target");

    if (target == "water") {
        if (action == "simulate") {
            m_state.waterLevel.store(5.8f);
            m_state.alarmSoundActive.store(true);
            m_state.alarmCenterActive.store(true);
            m_state.waterControlActive.store(true);
            APP_LOG_WARNING("水浸风险警报: 检测到水位异常(5.8cm), 触发高风险评估");
            addLog("alarm", "水浸风险警报",
                "检测到水位异常(5.8cm), 触发安防风险评估(高风险), 联动报警服务和防护处置服务");
            addLog("warning", "防护处置启动",
                "水浸联动服务: 排水设备启动, 关闭相关水源阀门");
            addLog("warning", "报警服务启动",
                "声光报警: 蜂鸣器长鸣, LED灯闪烁; 指挥中心报警: 系统后台发出警报");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"水浸异常模拟已触发\"}");
        } else if (action == "reset") {
            m_state.waterLevel.store(0.2f);
            m_state.alarmSoundActive.store(false);
            m_state.alarmCenterActive.store(false);
            m_state.waterControlActive.store(false);
            APP_LOG_INFO("水浸风险解除: 水位恢复正常(0.2cm)");
            addLog("normal", "水浸风险解除",
                "水位恢复正常(0.2cm), 风险等级降低, 关闭报警和处置服务");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"水浸已恢复正常\"}");
        }
    } else if (target == "intrusion") {
        if (action == "simulate") {
            m_state.alarmSoundActive.store(true);
            m_state.alarmCenterActive.store(true);
            APP_LOG_WARNING("人员入侵警报: 红外探测器识别非法闯入行为");
            addLog("alarm", "人员入侵警报",
                "红外探测器识别非法闯入行为, 触发安防风险评估(高风险)");
            addLog("warning", "报警服务启动",
                "声光报警: 现场警示; 指挥中心报警: 通知管理员紧急处置");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"入侵模拟已触发\"}");
        } else if (action == "reset") {
            m_state.alarmSoundActive.store(false);
            m_state.alarmCenterActive.store(false);
            APP_LOG_INFO("入侵警报解除: 红外探测器确认无非法入侵");
            addLog("normal", "入侵警报解除",
                "红外探测器确认无非法入侵, 风险等级降低, 关闭报警服务");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"入侵警报已解除\"}");
        }
    } else if (target == "gas") {
        if (action == "simulate") {
            m_state.gasSimulated.store(true);
            m_state.simTvoc.store(680);   // >500 ppb 高风险
            m_state.simCh2o.store(150);   // >100 ppb 高风险
            m_state.simO3.store(130);     // >100 ppb 高风险
            m_state.simCo2.store(1200);   // >1000 ppm 高风险
            m_state.alarmSoundActive.store(true);
            m_state.alarmCenterActive.store(true);
            m_state.ventilationActive.store(true);
            APP_LOG_WARNING("有害气体警报: 检测到多种有害气体浓度超限(TVOC:680ppb CH2O:150ppb O3:130ppb CO2:1200ppm)");
            addLog("alarm", "有害气体警报",
                "检测到多种有害气体浓度超限, 触发安防风险评估(高风险)");
            addLog("warning", "防护处置启动",
                "通风设备服务: 自动启动通风系统, 降低危害");
            addLog("warning", "报警服务启动",
                "声光报警和指挥中心报警已激活");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"气体泄漏模拟已触发\"}");
        } else if (action == "reset") {
            m_state.gasSimulated.store(false);
            m_state.simTvoc.store(0);
            m_state.simCh2o.store(0);
            m_state.simO3.store(0);
            m_state.simCo2.store(0);
            m_state.alarmSoundActive.store(false);
            m_state.alarmCenterActive.store(false);
            m_state.ventilationActive.store(false);
            APP_LOG_INFO("气体浓度恢复正常: 已取消模拟, 读取真实传感器数据");
            addLog("normal", "气体浓度恢复正常",
                "有害气体模拟已取消, 恢复读取真实传感器数据");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"气体浓度已恢复正常\"}");
        }
    }

    return HttpResponse::error(400, "无效的控制指令");
}

HttpResponse AppSecurity::handleGetLogs(const HttpRequest& req) {
    int limit = 50;
    std::string limitStr = req.getParam("limit");
    if (!limitStr.empty()) {
        limit = std::stoi(limitStr);
    }

    std::lock_guard<std::mutex> lock(m_logMutex);
    std::string json = "[";
    int count = 0;
    // 返回最新的日志 (从后往前)
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
void AppSecurity::evaluateRisk(int waterState, int irState, int radarState) {
    // 检查设备在线状态
    bool waterOnline = dev_water.isOnline();
    bool irOnline = dev_infrared.isOnline();

    m_state.waterRisk = calcWaterRisk(m_state.waterLevel.load(), waterState);
    m_state.intrusionRisk = calcIntrusionRisk(irState, radarState);

    // 气体风险: 基于云测仪真实传感器 (或模拟值)
    SecurityRiskLevel maxGasRisk = SecurityRiskLevel::LOW;
    if (m_state.gasSimulated.load()) {
        maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(m_state.simTvoc.load(), 200, 500));
        maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(m_state.simCh2o.load(), 50, 100));
        maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(m_state.simO3.load(), 60, 100));
        maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(m_state.simCo2.load(), 800, 1000));
    } else {
        if (dev_tvoc.isOnline()) maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(dev_tvoc.getValue(), 200, 500));
        if (dev_ch2o.isOnline()) maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(dev_ch2o.getValue(), 50, 100));
        if (dev_o3.isOnline())   maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(dev_o3.getValue(), 60, 100));
        if (dev_co2.isOnline())  maxGasRisk = std::max(maxGasRisk, calcSingleGasRisk(dev_co2.getValue(), 800, 1000));
    }
    m_state.gasRisk = maxGasRisk;

    // 设备离线时, 对应子系统标记为中风险
    if (!waterOnline) m_state.waterRisk = SecurityRiskLevel::MEDIUM;
    if (!irOnline) m_state.intrusionRisk = SecurityRiskLevel::MEDIUM;

    bool gasOnline = dev_tvoc.isOnline() || dev_ch2o.isOnline() || dev_o3.isOnline() || dev_co2.isOnline();
    if (!gasOnline && !m_state.gasSimulated.load()) m_state.gasRisk = SecurityRiskLevel::MEDIUM;

    int maxRisk = std::max({
        static_cast<int>(m_state.waterRisk),
        static_cast<int>(m_state.intrusionRisk),
        static_cast<int>(m_state.gasRisk)
    });
    m_state.overallRisk = static_cast<SecurityRiskLevel>(maxRisk);

    // 设备离线或有风险时, 系统不正常
    bool allOnline = waterOnline && irOnline && gasOnline;
    m_state.systemNormal.store(maxRisk == 0 && allOnline);
}

SecurityRiskLevel AppSecurity::calcWaterRisk(float level, int sensorState) {
    if (sensorState != 0 || level > 3.0f) return SecurityRiskLevel::HIGH;
    if (level > 1.0f) return SecurityRiskLevel::MEDIUM;
    return SecurityRiskLevel::LOW;
}

SecurityRiskLevel AppSecurity::calcIntrusionRisk(int irState, int radarState) {
    if (irState != 0 || radarState != 0) return SecurityRiskLevel::HIGH;
    return SecurityRiskLevel::LOW;
}

SecurityRiskLevel AppSecurity::calcSingleGasRisk(int value, int mediumThreshold, int highThreshold) {
    if (value > highThreshold) return SecurityRiskLevel::HIGH;
    if (value > mediumThreshold) return SecurityRiskLevel::MEDIUM;
    return SecurityRiskLevel::LOW;
}

void AppSecurity::addLog(const std::string& level, const std::string& event, const std::string& details) {
    // 获取当前时间字符串
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

    // 限制日志数量
    if (m_logs.size() > 500) {
        m_logs.erase(m_logs.begin(), m_logs.begin() + 100);
    }
}

void AppSecurity::triggerAlarm(const std::string& type, const std::string& message) {
    if (type == "sound") {
        m_state.alarmSoundActive.store(true);
    } else if (type == "center") {
        m_state.alarmCenterActive.store(true);
    } else if (type == "ventilation") {
        m_state.ventilationActive.store(true);
    } else if (type == "water_control") {
        m_state.waterControlActive.store(true);
    }
    APP_LOG_WARNING("报警触发(%s): %s", type.c_str(), message.c_str());
    addLog("alarm", "报警触发 - " + type, message);
}

void AppSecurity::clearAlarm(const std::string& type) {
    if (type == "sound") {
        m_state.alarmSoundActive.store(false);
    } else if (type == "center") {
        m_state.alarmCenterActive.store(false);
    } else if (type == "ventilation") {
        m_state.ventilationActive.store(false);
    } else if (type == "water_control") {
        m_state.waterControlActive.store(false);
    }
    APP_LOG_INFO("报警解除(%s)", type.c_str());
    addLog("normal", "报警解除 - " + type, "报警状态已清除");
}