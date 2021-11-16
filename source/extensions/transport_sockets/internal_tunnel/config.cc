#include "source/extensions/transport_sockets/internal_tunnel/config.h"

#include "envoy/extensions/transport_sockets/internal_tunnel/v3/internal_tunnel.pb.h"
#include "envoy/extensions/transport_sockets/internal_tunnel/v3/internal_tunnel.pb.validate.h"
#include "envoy/registry/registry.h"

#include "source/common/config/utility.h"
#include "source/common/protobuf/utility.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace InternalTunnel {

Network::TransportSocketPtr InternalTunnelSocketFactory::createTransportSocket(
    Network::TransportSocketOptionsConstSharedPtr) const {
  return nullptr;
}

Network::TransportSocketFactoryPtr
UpstreamInternalTunnelSocketConfigFactory::createTransportSocketFactory(
    const Protobuf::Message& message,
    Server::Configuration::TransportSocketFactoryContext& context) {
  UNREFERENCED_PARAMETER(message);
  UNREFERENCED_PARAMETER(context);
  return std::make_unique<InternalTunnelSocketFactory>();
}

ProtobufTypes::MessagePtr UpstreamInternalTunnelSocketConfigFactory::createEmptyConfigProto() {
  return std::make_unique<
      envoy::extensions::transport_sockets::internal_tunnel::v3::InternalTunnel>();
}

REGISTER_FACTORY(UpstreamInternalTunnelSocketConfigFactory,
                 Server::Configuration::UpstreamTransportSocketConfigFactory);

} // namespace InternalTunnel
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
