#pragma once

#include <functional>

#include "envoy/api/api.h"
#include "envoy/api/v2/fcds.pb.h"
#include "envoy/api/v2/lds.pb.h"
#include "envoy/config/subscription.h"
#include "envoy/init/manager.h"
#include "envoy/server/listener_manager.h"
#include "envoy/stats/scope.h"

#include "common/common/logger.h"
#include "common/init/target_impl.h"

namespace Envoy {
namespace Server {

/**
 * Interface for an FCDS API provider.
 */
class FcdsApi {
public:
  virtual ~FcdsApi() {}

  /**
   * @return std::string the last received version by the xDS API for FCDS.
   */
  virtual std::string versionInfo() const PURE;
};

typedef std::unique_ptr<FcdsApi> FcdsApiPtr;
/**
 * FCDS API implementation that fetches via Subscription.
 */
class FcdsApiImpl : public FcdsApi,
                    Config::SubscriptionCallbacks,
                    Logger::Loggable<Logger::Id::upstream> {};
} // namespace Server
} // namespace Envoy