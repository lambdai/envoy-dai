#pragma once

#include <chrono>

#include "envoy/common/time.h"
#include "envoy/event/dispatcher.h"

namespace Envoy {
class TimeUtil {
public:
  static SchedulerTime currentSchedulerTime(Event::Dispatcher& dispatcher) {
    return SchedulerTime{dispatcher.timeSource().monotonicTime(),
                         static_cast<uint32_t>(dispatcher.pollCount())};
  }
};
} // namespace Envoy
