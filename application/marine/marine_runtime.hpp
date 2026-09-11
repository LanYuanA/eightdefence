#ifndef MARINE_RUNTIME_HPP
#define MARINE_RUNTIME_HPP

#include "marine_executor.hpp"
#include "marine_repository.hpp"

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <thread>
#include <memory>
#include <functional>

namespace marine {
class MotorAtomicService;

struct NodeRun { TaskNode node; NodeStatus status = NodeStatus::Waiting; uint64_t elapsedMs = 0; double progress = 0; ServiceResult result; };
struct RunEvent { uint64_t atMs = 0; std::string message; std::string kind; };
struct MarineRun { std::string id; std::string appId; Application app; RunStatus status = RunStatus::Idle; int stepIndex = 0; uint64_t elapsedMs = 0; std::map<std::string, NodeRun> nodes; std::vector<RunEvent> events; };

class MarineRuntime {
public:
    using MotorAuthorizer = std::function<bool(const MotorParameters&)>;
    MarineRuntime(MarineRepository& repository, MarineExecutor executor, std::shared_ptr<MotorAtomicService> motors = nullptr, MotorAuthorizer motorAuthorizer = {});
    ~MarineRuntime();
    void start();
    void stop();
    MarineRun createRun(const std::string& appId);
    std::string pauseRun(const std::string& runId);
    std::string resumeRun(const std::string& runId);
    std::string cancelRun(const std::string& runId);
    std::string updateMotor(const std::string& runId, const std::string& nodeId, const MotorParameters& parameters);
    std::string stopMotor(const std::string& runId, const std::string& nodeId);
    bool emergencyStopMotors();
    std::vector<MarineRun> listRuns() const;
    std::optional<MarineRun> getRun(const std::string& runId) const;
    void advanceForTest(uint64_t milliseconds);

private:
    MarineRepository& repository_;
    MarineExecutor executor_;
    std::shared_ptr<MotorAtomicService> motors_;
    MotorAuthorizer motorAuthorizer_;
    mutable std::mutex mutex_;
    std::map<std::string, MarineRun> runs_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    uint64_t nextRunId_ = 1;
    void workerLoop();
    void advanceLocked(uint64_t milliseconds);
    void startStep(MarineRun& run);
    void addEvent(MarineRun& run, const std::string& message, const std::string& kind = "info");
    void persistLocked();
    void stopMotors(MarineRun& run);
};

} // namespace marine

#endif
