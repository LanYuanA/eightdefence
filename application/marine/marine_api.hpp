#ifndef MARINE_API_HPP
#define MARINE_API_HPP

#include "application/app_base.hpp"
#include "marine_runtime.hpp"
#include "marine_safety.hpp"

namespace marine {
using HttpRequest = ::HttpRequest;
using HttpResponse = ::HttpResponse;

class MarineApi {
public:
    MarineApi(MarineRepository& repository, MarineRuntime& runtime, MarineSafety& safety);
    HttpResponse handle(const HttpRequest& request);
private:
    MarineRepository& repository_;
    MarineRuntime& runtime_;
    MarineSafety& safety_;
};
} // namespace marine
#endif
