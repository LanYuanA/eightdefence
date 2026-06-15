/**
 * @file app_environment.cpp
 * @brief 环境监测应用实现
 */

#include "app_environment.hpp"
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
AppEnvironment::AppEnvironment()
    : AppBase("environment", "/environment", "智能环境监测系统 - 温湿度/PM2.5/PM10/有害气体环境监测")
{
    registerApi("/api/status",  [this](const HttpRequest& r){ return handleGetStatus(r); });
    registerApi("/api/sensors", [this](const HttpRequest& r){ return handleGetSensors(r); });
    registerApi("/api/control", [this](const HttpRequest& r){ return handlePostControl(r); });
    registerApi("/api/logs",    [this](const HttpRequest& r){ return handleGetLogs(r); });
}

AppEnvironment::~AppEnvironment() {
    stop();
}

/* ============================================================
 * 生命周期
 * ============================================================ */
int AppEnvironment::init() {
    APP_LOG_INFO("初始化...");
    addLog("normal", "系统初始化", "环境监测应用正在初始化, 加载传感器配置");
    return 0;
}

int AppEnvironment::start() {
    AppBase::start();
    APP_LOG_INFO("启动, 路由前缀: %s", m_routePrefix.c_str());
    addLog("normal", "系统启动完成", "所有环境传感器初始化成功, 系统进入正常运行状态");
    return 0;
}

void AppEnvironment::stop() {
    if (m_running.load()) {
        AppBase::stop();
        APP_LOG_INFO("已停止");
    }
}

/* ============================================================
 * API 处理
 * ============================================================ */
HttpResponse AppEnvironment::handleApi(const HttpRequest& request) {
    for (const auto& route : m_apiRoutes) {
        if (request.path == route.first) {
            return route.second(request);
        }
    }
    return HttpResponse::error(404, "API not found: " + request.path);
}

HttpResponse AppEnvironment::handleGetStatus(const HttpRequest& /*req*/) {
    // 实时从设备读取数据 (温度值为实际值×10, 需除以10; 湿度同理)
    float temp = dev_temperature.isOnline() ? dev_temperature.getValue() / 10.0f : m_state.temperature.load();
    float hum  = dev_humidity.isOnline() ? dev_humidity.getValue() / 10.0f : m_state.humidity.load();
    float pm25Val = dev_pm25.isOnline() ? static_cast<float>(dev_pm25.getValue()) : m_state.pm25.load();
    float pm10Val = dev_pm10.isOnline() ? static_cast<float>(dev_pm10.getValue()) : m_state.pm10.load();

    // 更新状态缓存
    m_state.temperature.store(temp);
    m_state.humidity.store(hum);
    m_state.pm25.store(pm25Val);
    m_state.pm10.store(pm10Val);

    // 读取有害气体数据
    int tvocVal, ch2oVal, o3Val, co2Val;
    if (m_state.gasSimulated.load()) {
        tvocVal = m_state.simTvoc.load();
        ch2oVal = m_state.simCh2o.load();
        o3Val   = m_state.simO3.load();
        co2Val  = m_state.simCo2.load();
    } else {
        tvocVal = dev_tvoc.isOnline() ? dev_tvoc.getValue() : 0;
        ch2oVal = dev_ch2o.isOnline() ? dev_ch2o.getValue() : 0;
        o3Val   = dev_o3.isOnline()   ? dev_o3.getValue()   : 0;
        co2Val  = dev_co2.isOnline()  ? dev_co2.getValue()  : 0;
    }

    // 设备在线状态
    bool tempOnline    = dev_temperature.isOnline();
    bool humOnline     = dev_humidity.isOnline();
    bool pm25Online    = dev_pm25.isOnline();
    bool pm10Online    = dev_pm10.isOnline();
    bool tvocOnline    = dev_tvoc.isOnline();
    bool ch2oOnline    = dev_ch2o.isOnline();
    bool o3Online      = dev_o3.isOnline();
    bool co2Online     = dev_co2.isOnline();

    // 实时计算风险等级
    evaluateRisk();

    char json[4096];
    const char* riskStr[] = {"低风险", "中风险", "高风险"};

    snprintf(json, sizeof(json),
        "{"
        "\"system\":{"
        "\"overallRisk\":\"%s\","
        "\"systemNormal\":%s,"
        "\"running\":%s"
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
        "\"risk\":\"%s\","
        "\"online\":%s"
        "},"
        "\"airQuality\":{"
        "\"pm25\":%.1f,"
        "\"pm10\":%.1f,"
        "\"pm25Unit\":\"μg/m³\","
        "\"pm10Unit\":\"μg/m³\","
        "\"risk\":\"%s\","
        "\"pm25Online\":%s,"
        "\"pm10Online\":%s,"
        "\"airQualityActive\":%s"
        "},"
        "\"gas\":{"
        "\"tvoc\":{\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "\"ch2o\":{\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "\"o3\":{\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "\"co2\":{\"value\":%d,\"unit\":\"ppm\",\"online\":%s},"
        "\"risk\":\"%s\","
        "\"gasResponseActive\":%s,"
        "\"ventilationActive\":%s"
        "},"
        "\"services\":{"
        "\"tempControlActive\":%s,"
        "\"centerAlarmActive\":%s"
        "}"
        "}",
        riskStr[static_cast<int>(m_state.overallRisk)],
        m_state.systemNormal.load() ? "true" : "false",
        m_running.load() ? "true" : "false",
        temp,
        riskStr[static_cast<int>(m_state.tempRisk)],
        tempOnline ? "true" : "false",
        hum,
        riskStr[static_cast<int>(m_state.humRisk)],
        humOnline ? "true" : "false",
        pm25Val, pm10Val,
        riskStr[static_cast<int>(m_state.pmRisk)],
        pm25Online ? "true" : "false",
        pm10Online ? "true" : "false",
        m_state.airQualityActive.load() ? "true" : "false",
        tvocVal, tvocOnline ? "true" : "false",
        ch2oVal, ch2oOnline ? "true" : "false",
        o3Val,   o3Online   ? "true" : "false",
        co2Val,  co2Online  ? "true" : "false",
        riskStr[static_cast<int>(m_state.gasRisk)],
        m_state.gasResponseActive.load() ? "true" : "false",
        m_state.ventilationActive.load() ? "true" : "false",
        m_state.tempControlActive.load() ? "true" : "false",
        m_state.centerAlarmActive.load() ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppEnvironment::handleGetSensors(const HttpRequest& /*req*/) {
    float temp = dev_temperature.isOnline() ? dev_temperature.getValue() / 10.0f : 0.0f;
    float hum  = dev_humidity.isOnline() ? dev_humidity.getValue() / 10.0f : 0.0f;
    float pm25Val = dev_pm25.isOnline() ? static_cast<float>(dev_pm25.getValue()) : 0.0f;
    float pm10Val = dev_pm10.isOnline() ? static_cast<float>(dev_pm10.getValue()) : 0.0f;

    char json[2048];
    snprintf(json, sizeof(json),
        "["
        "{\"name\":\"温度传感器\",\"type\":\"temperature\",\"value\":%.1f,\"unit\":\"°C\",\"online\":%s},"
        "{\"name\":\"湿度传感器\",\"type\":\"humidity\",\"value\":%.1f,\"unit\":\"%%\",\"online\":%s},"
        "{\"name\":\"PM2.5传感器\",\"type\":\"pm25\",\"value\":%.1f,\"unit\":\"μg/m³\",\"online\":%s},"
        "{\"name\":\"PM10传感器\",\"type\":\"pm10\",\"value\":%.1f,\"unit\":\"μg/m³\",\"online\":%s},"
        "{\"name\":\"TVOC传感器\",\"type\":\"tvoc\",\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "{\"name\":\"甲醛传感器\",\"type\":\"ch2o\",\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "{\"name\":\"臭氧传感器\",\"type\":\"o3\",\"value\":%d,\"unit\":\"ppb\",\"online\":%s},"
        "{\"name\":\"CO2传感器\",\"type\":\"co2\",\"value\":%d,\"unit\":\"ppm\",\"online\":%s}"
        "]",
        temp, dev_temperature.isOnline() ? "true" : "false",
        hum,  dev_humidity.isOnline() ? "true" : "false",
        pm25Val, dev_pm25.isOnline() ? "true" : "false",
        pm10Val, dev_pm10.isOnline() ? "true" : "false",
        dev_tvoc.isOnline() ? dev_tvoc.getValue() : 0, dev_tvoc.isOnline() ? "true" : "false",
        dev_ch2o.isOnline() ? dev_ch2o.getValue() : 0, dev_ch2o.isOnline() ? "true" : "false",
        dev_o3.isOnline()   ? dev_o3.getValue()   : 0, dev_o3.isOnline()   ? "true" : "false",
        dev_co2.isOnline()  ? dev_co2.getValue()  : 0, dev_co2.isOnline()  ? "true" : "false"
    );

    return HttpResponse::json(json);
}

HttpResponse AppEnvironment::handlePostControl(const HttpRequest& req) {
    std::string action = req.getParam("action");
    std::string target = req.getParam("target");

    if (target == "gas") {
        if (action == "simulate") {
            m_state.gasSimulated.store(true);
            m_state.simTvoc.store(680);
            m_state.simCh2o.store(150);
            m_state.simO3.store(130);
            m_state.simCo2.store(1200);
            evaluateRisk();
            APP_LOG_WARNING("有害气体模拟: TVOC:680ppb CH2O:150ppb O3:130ppb CO2:1200ppm");
            addLog("alarm", "有害气体模拟", "模拟有害气体浓度超限已触发");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"有害气体模拟已触发\"}");
        } else if (action == "reset") {
            m_state.gasSimulated.store(false);
            m_state.simTvoc.store(0);
            m_state.simCh2o.store(0);
            m_state.simO3.store(0);
            m_state.simCo2.store(0);
            evaluateRisk();
            APP_LOG_INFO("有害气体模拟取消: 恢复读取真实传感器数据");
            addLog("normal", "气体浓度恢复正常", "有害气体模拟已取消");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"气体浓度已恢复正常\"}");
        }
    } else if (target == "air") {
        if (action == "simulate") {
            m_state.pm25.store(200.0f);
            m_state.pm10.store(350.0f);
            evaluateRisk();
            APP_LOG_WARNING("空气质量模拟: PM2.5:200 PM10:350");
            addLog("alarm", "空气质量模拟", "模拟PM2.5/PM10超标已触发");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"空气质量异常模拟已触发\"}");
        } else if (action == "reset") {
            m_state.pm25.store(0.0f);
            m_state.pm10.store(0.0f);
            evaluateRisk();
            APP_LOG_INFO("空气质量恢复正常");
            addLog("normal", "空气质量恢复正常", "PM2.5/PM10模拟已取消");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"空气质量已恢复正常\"}");
        }
    } else if (target == "temp") {
        if (action == "simulate") {
            m_state.temperature.store(38.0f);
            m_state.humidity.store(85.0f);
            evaluateRisk();
            APP_LOG_WARNING("温湿度模拟: 温度38°C 湿度85%%");
            addLog("alarm", "温湿度异常模拟", "模拟温度/湿度超标已触发");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"温湿度异常模拟已触发\"}");
        } else if (action == "reset") {
            m_state.temperature.store(25.0f);
            m_state.humidity.store(50.0f);
            evaluateRisk();
            APP_LOG_INFO("温湿度恢复正常");
            addLog("normal", "温湿度恢复正常", "温湿度模拟已取消");
            return HttpResponse::json("{\"status\":\"success\",\"message\":\"温湿度已恢复正常\"}");
        }
    }

    return HttpResponse::error(400, "无效的控制指令");
}

HttpResponse AppEnvironment::handleGetLogs(const HttpRequest& req) {
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
void AppEnvironment::evaluateRisk() {
    bool tempOnline = dev_temperature.isOnline();
    bool humOnline  = dev_humidity.isOnline();
    bool pm25Online = dev_pm25.isOnline();
    bool pm10Online = dev_pm10.isOnline();

    m_state.tempRisk = calcTempRisk(m_state.temperature.load());
    m_state.humRisk  = calcHumRisk(m_state.humidity.load());
    m_state.pmRisk   = calcPmRisk(m_state.pm25.load(), m_state.pm10.load());

    // 气体风险
    EnvRiskLevel maxGasRisk = EnvRiskLevel::LOW;
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

    // 设备离线时标记中风险
    if (!tempOnline) m_state.tempRisk = EnvRiskLevel::MEDIUM;
    if (!humOnline)  m_state.humRisk  = EnvRiskLevel::MEDIUM;
    if (!pm25Online && !pm10Online) m_state.pmRisk = EnvRiskLevel::MEDIUM;

    bool gasOnline = dev_tvoc.isOnline() || dev_ch2o.isOnline() || dev_o3.isOnline() || dev_co2.isOnline();
    if (!gasOnline && !m_state.gasSimulated.load()) m_state.gasRisk = EnvRiskLevel::MEDIUM;

    int maxRisk = std::max({
        static_cast<int>(m_state.tempRisk),
        static_cast<int>(m_state.humRisk),
        static_cast<int>(m_state.pmRisk),
        static_cast<int>(m_state.gasRisk)
    });
    m_state.overallRisk = static_cast<EnvRiskLevel>(maxRisk);

    bool allOnline = tempOnline && humOnline && (pm25Online || pm10Online) && gasOnline;
    m_state.systemNormal.store(maxRisk == 0 && allOnline);

    handleRiskResponse();
}

EnvRiskLevel AppEnvironment::calcTempRisk(float temp) {
    if (temp > 35.0f || temp < 5.0f) return EnvRiskLevel::HIGH;
    if (temp > 30.0f || temp < 10.0f) return EnvRiskLevel::MEDIUM;
    return EnvRiskLevel::LOW;
}

EnvRiskLevel AppEnvironment::calcHumRisk(float hum) {
    if (hum > 85.0f || hum < 20.0f) return EnvRiskLevel::HIGH;
    if (hum > 70.0f || hum < 30.0f) return EnvRiskLevel::MEDIUM;
    return EnvRiskLevel::LOW;
}

EnvRiskLevel AppEnvironment::calcPmRisk(float pm25, float pm10) {
    if (pm25 > 150.0f || pm10 > 300.0f) return EnvRiskLevel::HIGH;
    if (pm25 > 75.0f  || pm10 > 150.0f) return EnvRiskLevel::MEDIUM;
    return EnvRiskLevel::LOW;
}

EnvRiskLevel AppEnvironment::calcSingleGasRisk(int value, int mediumThreshold, int highThreshold) {
    if (value > highThreshold) return EnvRiskLevel::HIGH;
    if (value > mediumThreshold) return EnvRiskLevel::MEDIUM;
    return EnvRiskLevel::LOW;
}

void AppEnvironment::addLog(const std::string& level, const std::string& event, const std::string& details) {
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

void AppEnvironment::setServices(SvcTempHumidityControl* a1, SvcGasResponse* a2,
                                  SvcAirQualityAlert* a3, SvcVentilation* a4,
                                  SvcCommandCenter* a5) {
    m_svcTempHumid   = a1;
    m_svcGasResp     = a2;
    m_svcAirQuality  = a3;
    m_svcVentilation = a4;
    m_svcCmdCenter   = a5;
}

void AppEnvironment::handleRiskResponse() {
    // 温湿度风险: 启动温湿度调控服务
    if (m_state.tempRisk >= EnvRiskLevel::MEDIUM || m_state.humRisk >= EnvRiskLevel::MEDIUM) {
        if (m_svcTempHumid) {
            m_svcTempHumid->activate();
            m_svcTempHumid->checkAndControl(m_state.temperature.load(), m_state.humidity.load());
        }
        m_state.tempControlActive.store(true);
        if (m_prevTempRisk < EnvRiskLevel::MEDIUM && m_prevHumRisk < EnvRiskLevel::MEDIUM) {
            addLog("warning", "温湿度调控启动",
                "温度或湿度超出正常范围, 温湿度调控服务已激活");
        }
    } else {
        if (m_svcTempHumid) m_svcTempHumid->deactivate();
        m_state.tempControlActive.store(false);
        if (m_prevTempRisk >= EnvRiskLevel::MEDIUM || m_prevHumRisk >= EnvRiskLevel::MEDIUM) {
            addLog("normal", "温湿度调控停止", "温湿度恢复正常, 调控服务已停止");
        }
    }

    // 空气质量风险: 启动空气质量告警服务
    if (m_state.pmRisk >= EnvRiskLevel::MEDIUM) {
        if (m_svcAirQuality) {
            m_svcAirQuality->activate();
            m_svcAirQuality->checkAndControl(m_state.pm25.load(), m_state.pm10.load());
        }
        m_state.airQualityActive.store(true);
        if (m_prevPmRisk < EnvRiskLevel::MEDIUM) {
            addLog("warning", "空气质量告警", "PM2.5/PM10 超标, 空气质量告警服务已激活");
        }
    } else {
        if (m_svcAirQuality) m_svcAirQuality->deactivate();
        m_state.airQualityActive.store(false);
        if (m_prevPmRisk >= EnvRiskLevel::MEDIUM) {
            addLog("normal", "空气质量恢复", "PM2.5/PM10 恢复正常, 告警服务已停止");
        }
    }

    // 有害气体风险: 启动气体处理 + 通风服务
    if (m_state.gasRisk >= EnvRiskLevel::HIGH) {
        if (m_svcGasResp) m_svcGasResp->activate();
        if (m_svcVentilation) m_svcVentilation->activate();
        if (m_svcCmdCenter) {
            m_svcCmdCenter->activate();
            m_svcCmdCenter->alert("high", "有害气体警报", "有害气体浓度超限, 净化和通风系统已启动");
        }
        m_state.gasResponseActive.store(true);
        m_state.ventilationActive.store(true);
        m_state.centerAlarmActive.store(true);
        if (m_prevGasRisk < EnvRiskLevel::HIGH) {
            addLog("alarm", "有害气体警报", "有害气体浓度超限, 联动净化+通风+指挥中心");
        }
    } else if (m_state.gasRisk >= EnvRiskLevel::MEDIUM) {
        if (m_svcGasResp) m_svcGasResp->activate();
        if (m_svcVentilation) m_svcVentilation->deactivate();
        m_state.gasResponseActive.store(true);
        m_state.ventilationActive.store(false);
        if (m_prevGasRisk < EnvRiskLevel::MEDIUM) {
            addLog("warning", "有害气体预警", "有害气体浓度偏高, 净化服务已启动");
        }
    } else {
        if (m_svcGasResp) m_svcGasResp->deactivate();
        if (m_svcVentilation) m_svcVentilation->deactivate();
        if (m_svcCmdCenter) m_svcCmdCenter->deactivate();
        m_state.gasResponseActive.store(false);
        m_state.ventilationActive.store(false);
        m_state.centerAlarmActive.store(false);
        if (m_prevGasRisk >= EnvRiskLevel::MEDIUM) {
            addLog("normal", "气体浓度恢复", "有害气体浓度恢复正常, 服务已停止");
        }
    }

    m_prevTempRisk = m_state.tempRisk;
    m_prevHumRisk  = m_state.humRisk;
    m_prevPmRisk   = m_state.pmRisk;
    m_prevGasRisk  = m_state.gasRisk;
}
