#include "extensions/upstreams/http/cluster_sensitive/config.h"

#include "extensions/upstreams/http/cluster_sensitive/http_upstream_request.h"
#include "extensions/upstreams/http/tcp/upstream_request.h"

namespace Envoy {
namespace Extensions {
namespace Upstreams {
namespace Http {
namespace ClusterSensitive {

Router::GenericConnPoolPtr ClusterSensitiveGenericConnPoolFactory::createGenericConnPool(
    Upstream::ClusterManager& cm, bool is_connect, const Router::RouteEntry& route_entry,
    absl::optional<Envoy::Http::Protocol> downstream_protocol,
    Upstream::LoadBalancerContext* ctx) const {
  if (is_connect) {
    auto ret = std::make_unique<Upstreams::Http::Tcp::TcpConnPool>(cm, is_connect, route_entry,
                                                                   downstream_protocol, ctx);
    return (ret->valid() ? std::move(ret) : nullptr);
  }
  auto ret = std::make_unique<Upstreams::Http::ClusterSensitive::HttpConnPool>(
      cm, is_connect, route_entry, downstream_protocol, ctx);
  return (ret->valid() ? std::move(ret) : nullptr);
}

REGISTER_FACTORY(ClusterSensitiveGenericConnPoolFactory, Router::GenericConnPoolFactory);

} // namespace ClusterSensitive
} // namespace Http
} // namespace Upstreams
} // namespace Extensions
} // namespace Envoy
