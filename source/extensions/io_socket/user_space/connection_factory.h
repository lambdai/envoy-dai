#pragma once

#include "envoy/network/address.h"

#include "common/network/connection_factory.h"

namespace Envoy {
namespace Extensions {
namespace IoSocket {
namespace UserSpace {
class ClientConnectionFactoryImpl : public Network::ClientConnectionFactory {
public:
  /**
   * Create a particular client connection.
   * @return ClientConnectionPtr the client connection.
   */
  Network::ClientConnectionPtr
  createClientConnection(Event::Dispatcher& dispatcher,
                         Network::Address::InstanceConstSharedPtr address,
                         Network::Address::InstanceConstSharedPtr source_address,
                         Network::TransportSocketPtr transport_socket,
                         const Network::ConnectionSocket::OptionsSharedPtr& options) override;

  std::string name() const override { return std::string(Network::Address::EnvoyInternalName); }
};
} // namespace UserSpace
} // namespace IoSocket
} // namespace Extensions
} // namespace Envoy