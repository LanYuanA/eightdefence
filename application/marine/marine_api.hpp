#ifndef MARINE_API_HPP
#define MARINE_API_HPP

#include "application/app_base.hpp"
#include "marine_runtime.hpp"
#include "marine_safety.hpp"
#include "service/atomic/svc_motor_speed.hpp"

namespace marine {
using HttpRequest = ::HttpRequest;
using HttpResponse = ::HttpResponse;

class MarineApi {
public:
    MarineApi(MarineRepository& repository, MarineRuntime& runtime, MarineSafety& safety, std::shared_ptr<MotorAtomicService> motors = nullptr);
    HttpResponse handle(const HttpRequest& request);
private:
    MarineRepository& repository_;
    MarineRuntime& runtime_;
    MarineSafety& safety_;
    std::shared_ptr<MotorAtomicService> motors_;
};
} // namespace marine
#endif
