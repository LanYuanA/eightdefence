/**
 * @file svc_drainage.hpp
 * @brief 原子服务 - 排水控制（模拟）
 */

#ifndef SVC_DRAINAGE_HPP
#define SVC_DRAINAGE_HPP

#include "svc_base.hpp"
#include <atomic>

class SvcDrainage : public SvcBase {
public:
    std::string getName() const override { return "排水控制服务"; }
    void activate() override;
    void deactivate() override;
    bool isActive() const override { return active_.load(); }

private:
    std::atomic<bool> active_{false};
};

#endif /* SVC_DRAINAGE_HPP */
