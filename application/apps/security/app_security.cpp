/**
 * @file app_security.cpp
 * @brief 安防系统应用实现
 */

#include "app_security.hpp"
#include "../../../core/global_devices.hpp"
#include "../../../core/logger.hpp"

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
    LOG_INFO("[安防应用] 初始化...");
    addLog("normal", "系统初始化", "安防应用正在初始化, 加载传感器配置");
    return 0;
}

int AppSecurity::start() {
    AppBase::start();
    LOG_INFO("[安防应用] 启动, 路由前缀: %s", m_routePrefix.c_str());

    // 启动业务轮询线程
    m_pollThread = std::thread(&AppSecurity::pollingThread, this);

    addLog("normal", "系统启动完成", "所有安防传感器初始化成功, 系统进入正常运行状态");
    return 0;
}

void AppSecurity::stop() {
    if (m_running.load()) {
        AppBase::stop();
        LOG_INFO("[安防应用] 正在停止...");
        if (m_pollThread.joinable()) {
            m_pollThread.join();
        }
        LOG_INFO("[安防应用] 已停止");
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
    char json[2048];
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
        "\"controlActive\":%s"
        "},"
        "\"intrusion\":{"
        "\"infraredState\":%d,"
        "\"radarState\":%d,"
        "\"risk\":\"%s\""
        "},"
        "\"gas\":{"
        "\"concentration\":%d,"
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
        m_state.waterLevel.load(),
        m_state.waterSensorState.load(),
        riskStr[static_cast<int>(m_state.waterRisk)],
        m_state.waterControlActive.load() ? "true" : "false",
        m_state.infraredState.load(),
        m_state.radarState.load(),
        riskStr[static_cast<int>(m_state.intrusionRisk)],
        m_state.gasConcentration.load(),
        riskStr[static_cast<int>(m_state.gasRisk)],
        m_state.ventilationActive.load() ? "true" : "false",
        m_state.alarmSoundActive.load() ? "true" : "false",
        m_state.alarmCenterActive.load() ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppSecurity::handleGetSensors(const HttpRequest& /*req*/) {
    char json[1024];
    snprintf(json, sizeof(json),
        "["
        "{\"name\":\"水浸传感器\",\"type\":\"water\",\"state\":%d,\"value\":%.1f,\"unit\":\"cm\"},"
        "{\"name\":\"红外探测器\",\"type\":\"infrared\",\"state\":%d,\"value\":%d,\"unit\":\"\"},"
        "{\"name\":\"雷达探测器\",\"type\":\"radar\",\"state\":%d,\"value\":%d,\"unit\":\"\"},"
        "{\"name\":\"有害气体探测器\",\"type\":\"gas\",\"state\":0,\"value\":%d,\"unit\":\"PPM\"}"
        "]",
        m_state.waterSensorState.load(), m_state.waterLevel.load(),
        m_state.infraredState.load(), m_state.infraredState.load(),
        m_state.radarState.load(), m_state.radarState.load(),
        m_state.gasConcentration.load()
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
            m_state.waterSensorState.store(1);
            m_state.alarmSoundActive.store(true);
            m_state.alarmCenterActive.store(true);
            m_state.waterControlActive.store(true);
            addLog("alarm", "水浸风险警报",
                "检测到水位异常(5.8cm), 触发安防风险评估(高风险), 联动报警服务和防护处置服务");
            addLog("warning", "防护处置启动",
                "水浸联动服务: 排水设备启动, 关闭相关水源阀门");
            addLog("warning", "报警服务启动",
                "声光报警: 蜂鸣器长鸣, LED灯闪烁; 指挥中心报警: 系统后台发出警报");
            evaluateRisk();
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"水浸异常模拟已触发\"}");
        } else if (action == "reset") {
            m_state.waterLevel.store(0.2f);
            m_state.waterSensorState.store(0);
            m_state.alarmSoundActive.store(false);
            m_state.alarmCenterActive.store(false);
            m_state.waterControlActive.store(false);
            addLog("normal", "水浸风险解除",
                "水位恢复正常(0.2cm), 风险等级降低, 关闭报警和处置服务");
            evaluateRisk();
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"水浸已恢复正常\"}");
        }
    } else if (target == "intrusion") {
        if (action == "simulate") {
            m_state.infraredState.store(1);
            m_state.radarState.store(1);
            m_state.alarmSoundActive.store(true);
            m_state.alarmCenterActive.store(true);
            addLog("alarm", "人员入侵警报",
                "红外探测器识别非法闯入行为, 触发安防风险评估(高风险)");
            addLog("warning", "报警服务启动",
                "声光报警: 现场警示; 指挥中心报警: 通知管理员紧急处置");
            evaluateRisk();
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"入侵模拟已触发\"}");
        } else if (action == "reset") {
            m_state.infraredState.store(0);
            m_state.radarState.store(0);
            m_state.alarmSoundActive.store(false);
            m_state.alarmCenterActive.store(false);
            addLog("normal", "入侵警报解除",
                "红外探测器确认无非法入侵, 风险等级降低, 关闭报警服务");
            evaluateRisk();
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"入侵警报已解除\"}");
        }
    } else if (target == "gas") {
        if (action == "simulate") {
            m_state.gasConcentration.store(156);
            m_state.alarmSoundActive.store(true);
            m_state.alarmCenterActive.store(true);
            m_state.ventilationActive.store(true);
            addLog("alarm", "有害气体警报",
                "检测到有害气体浓度超限(156 PPM), 触发安防风险评估(高风险)");
            addLog("warning", "防护处置启动",
                "通风设备服务: 自动启动通风系统, 降低危害");
            addLog("warning", "报警服务启动",
                "声光报警和指挥中心报警已激活");
            evaluateRisk();
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"气体泄漏模拟已触发\"}");
        } else if (action == "reset") {
            m_state.gasConcentration.store(12);
            m_state.alarmSoundActive.store(false);
            m_state.alarmCenterActive.store(false);
            m_state.ventilationActive.store(false);
            addLog("normal", "气体浓度恢复正常",
                "有害气体浓度降至安全范围(12 PPM), 关闭报警和通风设备");
            evaluateRisk();
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
void AppSecurity::pollingThread() {
    LOG_INFO("[安防应用] 轮询线程启动");

    while (m_running.load()) {
        // 从实际硬件读取数据 (通过全局设备实例)
        // 水浸传感器
        m_state.waterSensorState.store(dev_water.getWaterState());
        // 红外/雷达
        m_state.infraredState.store(dev_infrared.getInfraredState());
        m_state.radarState.store(dev_infrared.getRadarState());

        // 风险评估
        evaluateRisk();

        // 每 2 秒轮询一次
        for (int i = 0; i < 20 && m_running.load(); i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    LOG_INFO("[安防应用] 轮询线程退出");
}

void AppSecurity::evaluateRisk() {
    updateWaterRisk();
    updateIntrusionRisk();
    updateGasRisk();

    // 综合风险 = 最高单项风险
    int maxRisk = std::max({
        static_cast<int>(m_state.waterRisk),
        static_cast<int>(m_state.intrusionRisk),
        static_cast<int>(m_state.gasRisk)
    });
    m_state.overallRisk = static_cast<SecurityRiskLevel>(maxRisk);
    m_state.systemNormal.store(maxRisk == 0);
}

void AppSecurity::updateWaterRisk() {
    float level = m_state.waterLevel.load();
    int state = m_state.waterSensorState.load();

    if (state != 0 || level > 3.0f) {
        m_state.waterRisk = SecurityRiskLevel::HIGH;
    } else if (level > 1.0f) {
        m_state.waterRisk = SecurityRiskLevel::MEDIUM;
    } else {
        m_state.waterRisk = SecurityRiskLevel::LOW;
    }
}

void AppSecurity::updateIntrusionRisk() {
    int ir = m_state.infraredState.load();
    int radar = m_state.radarState.load();

    if (ir != 0 || radar != 0) {
        m_state.intrusionRisk = SecurityRiskLevel::HIGH;
    } else {
        m_state.intrusionRisk = SecurityRiskLevel::LOW;
    }
}

void AppSecurity::updateGasRisk() {
    int concentration = m_state.gasConcentration.load();

    if (concentration > 100) {
        m_state.gasRisk = SecurityRiskLevel::HIGH;
    } else if (concentration > 50) {
        m_state.gasRisk = SecurityRiskLevel::MEDIUM;
    } else {
        m_state.gasRisk = SecurityRiskLevel::LOW;
    }
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
    addLog("normal", "报警解除 - " + type, "报警状态已清除");
}