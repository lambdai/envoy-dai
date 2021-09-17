#include "source/common/network/internal_socket_option_impl.h"

#include "envoy/common/exception.h"
#include "envoy/common/platform.h"
#include "envoy/config/core/v3/base.pb.h"

#include "source/common/api/os_sys_calls_impl.h"
#include "source/common/common/assert.h"
#include "source/common/network/address_impl.h"
#include "source/common/network/socket_option_impl.h"

namespace Envoy {
namespace Network {

bool InternalSocketOptionImpl::setOption(Socket& socket,
                                         // NOT ussed
                                         envoy::config::core::v3::SocketOption::SocketState) const {
  socket.connectionInfoProvider().setLocalAddress(original_local_address_);
  socket.connectionInfoProvider().setRemoteAddress(original_remote_address_);
  return true;
}

absl::optional<Socket::Option::Details> InternalSocketOptionImpl::getOptionDetails(
    const Socket&, envoy::config::core::v3::SocketOption::SocketState) const {
  //   if (!option.has_value()) {
  //     return absl::nullopt;
  //   }

  //   return option->get().getOptionDetails(socket, state);
  return absl::nullopt;
}

} // namespace Network
} // namespace Envoy
