#pragma once

#include "envoy/network/address.h"
#include "envoy/network/connection.h"
#include "envoy/network/listen_socket.h"
#include "envoy/network/transport_socket.h"

namespace Envoy {
namespace Network {

class ClientConnectionFactory {
public:
  virtual ~ClientConnectionFactory() = default;
  std::string category() { return "network.connection"; }
  virtual std::string name() PURE;

  virtual Network::ClientConnectionPtr
  createClientConnection(Event::Dispatcher& dispatcher,
                         Network::Address::InstanceConstSharedPtr address,
                         Network::Address::InstanceConstSharedPtr source_address,
                         Network::TransportSocketPtr&& transport_socket,
                         const Network::ConnectionSocket::OptionsSharedPtr& options) PURE;
};

} // namespace Network
} // namespace Envoy
