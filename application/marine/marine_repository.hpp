#ifndef MARINE_REPOSITORY_HPP
#define MARINE_REPOSITORY_HPP

#include "marine_types.hpp"

#include <string>
#include <vector>

namespace marine {

class MarineRepository {
public:
    explicit MarineRepository(std::string root);
    std::string load();
    const std::vector<Application>& listApps() const;
    std::string saveApp(const Application& app);
    std::string removeApp(const std::string& appId);
    const std::vector<Binding>& listBindings() const;
    std::string updateBinding(const Binding& binding, uint64_t expectedVersion, const std::string& operatorName);
    const std::vector<RunSummary>& listRunSummaries() const;
    std::string saveRunSummaries(const std::vector<RunSummary>& runs);
    std::string appendEvent(const std::string& type, const std::string& message);

private:
    std::string root_;
    std::vector<Application> apps_;
    std::vector<Binding> bindings_;
    std::vector<RunSummary> runs_;
    std::string readApps();
    std::string readBindings();
    std::string readRuns();
    std::string writeApps();
    std::string writeBindings();
    std::string writeRuns();
};

} // namespace marine

#endif
