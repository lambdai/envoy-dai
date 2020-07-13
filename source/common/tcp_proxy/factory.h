#pragma once

#include "common/tcp_proxy/upstream_interface.h"

namespace Envoy {

namespace Upstream {
class LoadBalancerContext;
class ClusterManager;
} // namespace Upstream

namespace TcpProxy {

// TODO(lambdai): move to include path.
class GenericConnPoolFactory {
public:
  virtual ~GenericConnPoolFactory() = default;
  virtual GenericConnPoolPtr createGenericConnPool(
      Envoy::Upstream::ClusterManager& cluster_manager, absl::string_view hostname,
      const std::string& cluster_name, Envoy::Upstream::LoadBalancerContext* lb_context,
      Envoy::TcpProxy::GenericUpstreamPoolCallbacks& generic_pool_callback,
      const std::shared_ptr<Envoy::Tcp::ConnectionPool::UpstreamCallbacks>& upstream_callback)
      const PURE;
};

using GenericConnPoolFactorySharedPtr = std::shared_ptr<GenericConnPoolFactory>;

} // namespace TcpProxy
} // namespace Envoy
