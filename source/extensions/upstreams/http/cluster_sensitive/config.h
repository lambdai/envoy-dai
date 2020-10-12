#pragma once

#include "envoy/extensions/upstreams/http/http/v3/http_connection_pool.pb.h"
#include "envoy/registry/registry.h"
#include "envoy/router/router.h"

namespace Envoy {
namespace Extensions {
namespace Upstreams {
namespace Http {
namespace ClusterSensitive {

/**
 * Config registration for the HttpConnPool. @see Router::GenericConnPoolFactory
 */
class ClusterSensitiveGenericConnPoolFactory : public Router::GenericConnPoolFactory {
public:
  std::string name() const override {
    return "envoy.filters.connection_pools.http.cluster_senstive";
  }
  std::string category() const override { return "envoy.upstreams"; }
  Router::GenericConnPoolPtr
  createGenericConnPool(Upstream::ClusterManager& cm, bool is_connect,
                        const Router::RouteEntry& route_entry,
                        absl::optional<Envoy::Http::Protocol> downstream_protocol,
                        Upstream::LoadBalancerContext* ctx) const override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<
        envoy::extensions::upstreams::http::http::v3::HttpConnectionPoolProto>();
  }
};

DECLARE_FACTORY(ClusterSensitiveGenericConnPoolFactory);

} // namespace ClusterSensitive
} // namespace Http
} // namespace Upstreams
} // namespace Extensions
} // namespace Envoy
