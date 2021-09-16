#pragma once

#include "envoy/common/platform.h"
#include "envoy/config/core/v3/base.pb.h"
#include "envoy/network/listen_socket.h"

#include "source/common/common/logger.h"
#include "source/common/network/socket_option_impl.h"

#include "absl/types/optional.h"

namespace Envoy {
namespace Network {

class InternalSocketOptionImpl : public Socket::Option, Logger::Loggable<Logger::Id::connection> {
public:
  InternalSocketOptionImpl(const Network::Address::InstanceConstSharedPtr& remote_address,
                           const Network::Address::InstanceConstSharedPtr& local_address)
      : original_remote_address_(remote_address), original_local_address_(local_address) {}

  // Socket::Option
  bool setOption(Socket& socket,
                 envoy::config::core::v3::SocketOption::SocketState state) const override;
  // The common socket options don't require a hash key.
  void hashKey(std::vector<uint8_t>&) const override {}

  absl::optional<Details>
  getOptionDetails(const Socket& socket,
                   envoy::config::core::v3::SocketOption::SocketState state) const override;

private:
  const Network::Address::InstanceConstSharedPtr original_remote_address_;
  const Network::Address::InstanceConstSharedPtr original_local_address_;
};

} // namespace Network
} // namespace Envoy
