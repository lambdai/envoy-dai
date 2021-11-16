#include "source/extensions/transport_sockets/internal_tunnel/tunnel_socket.h"

#include "envoy/network/transport_socket.h"

#include "source/common/network/raw_buffer_socket.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace InternalTunnel {

UpstreamInternalTunnelSocket::UpstreamInternalTunnelSocket(
    Network::TransportSocketPtr&& transport_socket,
    Network::TransportSocketOptionsConstSharedPtr options)
    : PassthroughSocket(std::move(transport_socket)), options_(options) {}

void UpstreamInternalTunnelSocket::setTransportSocketCallbacks(
    Network::TransportSocketCallbacks& callbacks) {
  transport_socket_->setTransportSocketCallbacks(callbacks);
  callbacks_ = &callbacks;
}

UpstreamInternalTunnelSocketFactory::UpstreamInternalTunnelSocketFactory(
    Network::TransportSocketFactoryPtr transport_socket_factory, const InternalTunnelConfig& config)
    : // TODO: add back when when need inner tarnsport socket than raw buffer.
      // transport_socket_factory_(std::move(transport_socket_factory)),
      config_(config) {
  UNREFERENCED_PARAMETER(transport_socket_factory);
}

Network::TransportSocketPtr UpstreamInternalTunnelSocketFactory::createTransportSocket(
    Network::TransportSocketOptionsConstSharedPtr options) const {
  return std::make_unique<UpstreamInternalTunnelSocket>(
      std::make_unique<Network::RawBufferSocket>(), options);
}

} // namespace InternalTunnel
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
