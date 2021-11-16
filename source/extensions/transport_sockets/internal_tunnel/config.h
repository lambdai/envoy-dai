#pragma once

#include "envoy/server/transport_socket_config.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace InternalTunnel {

class InternalTunnelSocketFactory : public Network::TransportSocketFactory {
public:
  explicit InternalTunnelSocketFactory() = default;

  // Network::TransportSocketFactory
  bool implementsSecureTransport() const override { return false; }

  Network::TransportSocketPtr
  createTransportSocket(Network::TransportSocketOptionsConstSharedPtr options) const override;

  bool usesProxyProtocolOptions() const override { return false; }

  bool supportsAlpn() const override { return false; }
};

class UpstreamInternalTunnelSocketConfigFactory
    : public Server::Configuration::UpstreamTransportSocketConfigFactory {
public:
  std::string name() const override { return "envoy.transport_sockets.internal_tunnel"; }

  Network::TransportSocketFactoryPtr createTransportSocketFactory(
      const Protobuf::Message& config,
      Server::Configuration::TransportSocketFactoryContext& context) override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override;
};

} // namespace InternalTunnel
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
