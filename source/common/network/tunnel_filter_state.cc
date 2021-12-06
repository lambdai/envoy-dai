#include "source/common/network/tunnel_filter_state.h"

#include "source/common/common/macros.h"

namespace Envoy {
namespace Network {

const std::string& TunnelFilterState::key() {
  CONSTRUCT_ON_FIRST_USE(std::string, "envoy.network.tunnel_options");
}

} // namespace Network
} // namespace Envoy
