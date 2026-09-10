#include "marine_runtime.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace marine {
MarineRuntime::MarineRuntime(MarineRepository& repository, MarineExecutor executor) : repository_(repository), executor_(std::move(executor)) {}
MarineRuntime::~MarineRuntime() { stop(); }
void MarineRuntime::start() { if (running_.exchange(true)) return; worker_ = std::thread(&MarineRuntime::workerLoop, this); }
void MarineRuntime::stop() { if (!running_.exchange(false)) return; if (worker_.joinable()) worker_.join(); }
void MarineRuntime::workerLoop() { while (running_) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); std::lock_guard<std::mutex> lock(mutex_); advanceLocked(100); } }

MarineRun MarineRuntime::createRun(const std::string& appId) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto app = std::find_if(repository_.listApps().begin(), repository_.listApps().end(), [&](const Application& current) { return current.id == appId; });
    if (app == repository_.listApps().end()) return {"", appId, {}, RunStatus::Failed};
    MarineRun run; run.id = "RUN-" + std::to_string(nextRunId_++); run.appId = appId; run.app = *app; run.status = RunStatus::Running;
    for (const auto& step : run.app.steps) for (const auto& node : step.nodes) run.nodes.emplace(node.id, NodeRun{node});
    addEvent(run, "任务「" + run.app.name + "」开始 · 后端模拟");
    startStep(run); const MarineRun created = run; runs_.emplace(run.id, std::move(run)); persistLocked(); return created;
}
std::string MarineRuntime::pauseRun(const std::string& runId) { std::lock_guard<std::mutex> lock(mutex_); const auto iterator = runs_.find(runId); if (iterator == runs_.end()) return "not_found"; if (iterator->second.status != RunStatus::Running) return "invalid_state"; iterator->second.status = RunStatus::Paused; addEvent(iterator->second, "任务已暂停"); persistLocked(); return ""; }
std::string MarineRuntime::resumeRun(const std::string& runId) { std::lock_guard<std::mutex> lock(mutex_); const auto iterator = runs_.find(runId); if (iterator == runs_.end()) return "not_found"; if (iterator->second.status != RunStatus::Paused) return "invalid_state"; iterator->second.status = RunStatus::Running; addEvent(iterator->second, "任务继续执行"); persistLocked(); return ""; }
std::string MarineRuntime::cancelRun(const std::string& runId) { std::lock_guard<std::mutex> lock(mutex_); const auto iterator = runs_.find(runId); if (iterator == runs_.end()) return "not_found"; if (iterator->second.status != RunStatus::Running && iterator->second.status != RunStatus::Paused) return "invalid_state"; iterator->second.status = RunStatus::Cancelled; for (auto& entry : iterator->second.nodes) if (entry.second.status == NodeStatus::Waiting || entry.second.status == NodeStatus::Running) entry.second.status = NodeStatus::Cancelled; addEvent(iterator->second, "任务已停止 · 未完成服务已取消"); persistLocked(); return ""; }
std::vector<MarineRun> MarineRuntime::listRuns() const { std::lock_guard<std::mutex> lock(mutex_); std::vector<MarineRun> values; for (const auto& entry : runs_) values.push_back(entry.second); return values; }
std::optional<MarineRun> MarineRuntime::getRun(const std::string& runId) const { std::lock_guard<std::mutex> lock(mutex_); const auto iterator = runs_.find(runId); return iterator == runs_.end() ? std::nullopt : std::optional<MarineRun>(iterator->second); }
void MarineRuntime::advanceForTest(uint64_t milliseconds) { std::lock_guard<std::mutex> lock(mutex_); advanceLocked(milliseconds); }

void MarineRuntime::startStep(MarineRun& run) {
    if (run.stepIndex < 0 || static_cast<size_t>(run.stepIndex) >= run.app.steps.size()) return;
    const auto& step = run.app.steps.at(static_cast<size_t>(run.stepIndex));
    addEvent(run, "步骤 " + std::to_string(run.stepIndex + 1) + " · " + (step.nodes.size() > 1 ? "并行执行" : "顺序执行"));
    const SensorSnapshot sensors = executor_.snapshot();
    for (const auto& node : step.nodes) { auto& current = run.nodes.at(node.id); current.status = NodeStatus::Running; current.result = executor_.execute(node, 0, sensors); addEvent(run, node.serviceId + " 开始 · " + node.area); }
}
void MarineRuntime::advanceLocked(uint64_t milliseconds) {
    bool changed = false;
    for (auto& entry : runs_) {
        MarineRun& run = entry.second; if (run.status != RunStatus::Running || milliseconds == 0) continue;
        uint64_t remaining = milliseconds;
        while (remaining > 0 && run.status == RunStatus::Running) {
            if (static_cast<size_t>(run.stepIndex) >= run.app.steps.size()) { run.status = RunStatus::Completed; changed = true; break; }
            const auto& step = run.app.steps.at(static_cast<size_t>(run.stepIndex));
            std::vector<NodeRun*> active;
            uint64_t delta = remaining;
            for (const auto& node : step.nodes) { auto& nodeRun = run.nodes.at(node.id); if (nodeRun.status == NodeStatus::Running) { active.push_back(&nodeRun); const uint64_t total = static_cast<uint64_t>(node.duration) * 1000; delta = std::min(delta, total - nodeRun.elapsedMs); } }
            if (active.empty()) { run.status = RunStatus::Failed; addEvent(run, "活动步骤没有可执行服务。", "error"); changed = true; break; }
            const SensorSnapshot sensors = executor_.snapshot();
            for (auto* nodeRun : active) { const uint64_t total = static_cast<uint64_t>(nodeRun->node.duration) * 1000; nodeRun->elapsedMs = std::min(total, nodeRun->elapsedMs + delta); nodeRun->progress = static_cast<double>(nodeRun->elapsedMs) / total; nodeRun->result = executor_.execute(nodeRun->node, nodeRun->progress, sensors); if (nodeRun->elapsedMs == total) { nodeRun->status = NodeStatus::Completed; addEvent(run, nodeRun->node.serviceId + " 完成", "success"); } }
            run.elapsedMs += delta; remaining -= delta; changed = true;
            bool stepComplete = true; for (const auto& node : step.nodes) if (run.nodes.at(node.id).status != NodeStatus::Completed) { stepComplete = false; break; }
            if (stepComplete) { if (static_cast<size_t>(run.stepIndex + 1) >= run.app.steps.size()) { run.status = RunStatus::Completed; addEvent(run, "所有步骤已完成 · 任务结果已生成", "success"); } else { ++run.stepIndex; startStep(run); } }
        }
    }
    if (changed) persistLocked();
}
void MarineRuntime::addEvent(MarineRun& run, const std::string& message, const std::string& kind) { run.events.push_back({run.elapsedMs, message, kind}); }
void MarineRuntime::persistLocked() { std::vector<RunSummary> summaries; for (const auto& entry : runs_) summaries.push_back({entry.second.id, entry.second.appId, entry.second.status, entry.second.stepIndex}); repository_.saveRunSummaries(summaries); }

} // namespace marine
