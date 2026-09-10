#include "marine_executor.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace marine {
namespace {
std::string decimal(double value, int precision) { std::ostringstream output; output << std::fixed << std::setprecision(precision) << value; return output.str(); }
double clamp(double value) { return std::max(0.0, std::min(1.0, value)); }
}

SensorSnapshot SensorSnapshot::demo() {
    SensorSnapshot snapshot; snapshot.source = "demo";
    snapshot.values = {{"temperature", 26.8}, {"humidity", 58}, {"pm25", 23}, {"pm10", 36}, {"tvoc", 0.18}, {"ch2o", 0.03}, {"co2", 680}, {"smoke", 0}, {"water", 0}, {"ir", 1}, {"radar", 1}, {"lux", 420}};
    for (const auto& entry : snapshot.values) snapshot.online.emplace(entry.first, false);
    return snapshot;
}
MarineExecutor::MarineExecutor(SensorReader sensorReader) : sensorReader_(std::move(sensorReader)) {}
SensorSnapshot MarineExecutor::snapshot() const { SensorSnapshot current = sensorReader_ ? sensorReader_() : SensorSnapshot{}; if (current.source.empty() || current.values.empty()) return SensorSnapshot::demo(); return current; }
ServiceResult MarineExecutor::execute(const TaskNode& node, double progress, const SensorSnapshot& sensors) const {
    const double completed = clamp(progress);
    if (isAwarenessService(node.serviceId)) {
        const std::map<std::string, std::pair<std::string, std::string>> mapping = {{"A01", {"温湿度", "temperature"}}, {"A02", {"颗粒物", "pm25"}}, {"A03", {"气体环境", "co2"}}, {"A04", {"烟雾状态", "smoke"}}, {"A05", {"舱底水浸", "water"}}, {"A06", {"舱室活动", "ir"}}, {"A07", {"光照强度", "lux"}}};
        const auto item = mapping.at(node.serviceId); const auto value = sensors.values.find(item.second);
        return {item.first, value == sensors.values.end() ? "—" : decimal(value->second, value->second == static_cast<int>(value->second) ? 0 : 1), "", node.area + "态势采样完成", sensors.source.empty() ? "demo" : sensors.source};
    }
    if (node.serviceId == "01") return {"换气进度", decimal(completed * 100, 0), "%", node.area + "换气 · 强度 " + std::to_string(node.intensity) + "%", "simulated"};
    if (node.serviceId == "02") return {"循环流量", decimal(48 * node.intensity / 100.0 * std::min(1.0, completed * 2), 1), "m³/h", node.area + (completed < .5 ? "冷却循环已启动" : "冷却循环稳定"), "simulated"};
    if (node.serviceId == "03") return {"供水压力", decimal(.5 * node.intensity / 100.0 * std::min(1.0, completed * 2), 2), "MPa", node.area + "供水增压", "simulated"};
    if (node.serviceId == "04") return {"剩余水位", decimal(std::max(5.0, 60 - 55 * completed * node.intensity / 100.0), 1), "cm", node.area + "定时排水", "simulated"};
    if (node.serviceId == "05") return {"报警状态", completed >= 1 ? "已就绪" : "联动检查中", "", node.area + "检修安全报警联动", "simulated"};
    return {"", "", "", "该服务尚未实现", "simulated"};
}
} // namespace marine
