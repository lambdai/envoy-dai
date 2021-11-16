#pragma once

#include "envoy/network/transport_socket.h"

#include "source/common/common/logger.h"
#include "source/extensions/transport_sockets/common/passthrough.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace InternalTunnel {

// TODO: expand while new fields are added to protobuf messsage.
class InternalTunnelConfig {};
class UpstreamInternalTunnelSocketFactory : public Network::TransportSocketFactory {
public:
  explicit UpstreamInternalTunnelSocketFactory(
      Network::TransportSocketFactoryPtr transport_socket_factory,
      const InternalTunnelConfig& config);

  // Network::TransportSocketFactory
  bool implementsSecureTransport() const override { return false; }

  Network::TransportSocketPtr
  createTransportSocket(Network::TransportSocketOptionsConstSharedPtr options) const override;

  bool usesProxyProtocolOptions() const override { return false; }

  bool supportsAlpn() const override { return false; }

  // TODO(lambdai): the life time of the config is not clear due to HappyEyeball client connection.
  // Maybe define it as shared ptr?
  InternalTunnelConfig config_;
};

class UpstreamInternalTunnelSocket : public TransportSockets::PassthroughSocket,
                                     public Logger::Loggable<Logger::Id::connection> {
public:
  UpstreamInternalTunnelSocket(Network::TransportSocketPtr&& transport_socket,
                               Network::TransportSocketOptionsConstSharedPtr options);

  void setTransportSocketCallbacks(Network::TransportSocketCallbacks& callbacks) override;

private:
  Network::TransportSocketOptionsConstSharedPtr options_;
  Network::TransportSocketCallbacks* callbacks_{};
};
} // namespace InternalTunnel
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
