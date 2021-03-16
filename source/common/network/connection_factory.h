#pragma once

#include <memory>
#include <string>

#include "envoy/config/typed_config.h"
#include "envoy/event/dispatcher.h"
#include "envoy/network/connection.h"
#include "envoy/registry/registry.h"

#include "common/common/assert.h"
#include "common/singleton/const_singleton.h"

namespace Envoy {
namespace Network {

class ClientConnectionFactory : public Envoy::Config::UntypedFactory {
public:
  /**
   * Create a client connection to the destination address.
   * @return ClientConnectionPtr the client connection.
   */
  virtual Network::ClientConnectionPtr
  createClientConnection(Event::Dispatcher& dispatcher,
                         Network::Address::InstanceConstSharedPtr address,
                         Network::Address::InstanceConstSharedPtr source_address,
                         Network::TransportSocketPtr transport_socket,
                         const Network::ConnectionSocket::OptionsSharedPtr& options) PURE;

  std::string category() const override { return "envoy.connection"; }
};

} // namespace Network
} // namespace Envoy