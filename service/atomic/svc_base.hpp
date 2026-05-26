/**
 * @file svc_base.hpp
 * @brief 原子服务基类
 */

#ifndef SVC_BASE_HPP
#define SVC_BASE_HPP

#include <string>

class SvcBase {
public:
    virtual ~SvcBase() = default;
    virtual std::string getName() const = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual bool isActive() const = 0;
};

#endif /* SVC_BASE_HPP */
