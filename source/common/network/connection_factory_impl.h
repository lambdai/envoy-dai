#pragma once

#include <memory>
#include <string>

#include "envoy/config/typed_config.h"
#include "envoy/event/dispatcher.h"
#include "envoy/event/timer.h"
#include "envoy/network/connection.h"
#include "envoy/registry/registry.h"

#include "common/common/assert.h"
#include "common/network/connection_impl.h"
#include "common/singleton/const_singleton.h"

namespace Envoy {
namespace Network {

class ConnectionFactoryImpl : public ConnectionFactory {
public:
  explicit ConnectionFactoryImpl(Event::Dispatcher& dispatcher) : dispatcher_(dispatcher) {}

  /**
   * Wraps an already-accepted socket in an instance of Envoy's server Network::Connection.
   * @param socket supplies an open file descriptor and connection metadata to use for the
   *        connection. Takes ownership of the socket.
   * @param transport_socket supplies a transport socket to be used by the connection.
   * @param stream_info info object for the server connection
   * @return Network::ConnectionPtr a server connection that is owned by the caller.
   */

  ServerConnectionPtr createServerConnection(ConnectionSocketPtr&& socket,
                                             TransportSocketPtr&& transport_socket,
                                             StreamInfo::StreamInfo& stream_info) override;

  /**
   * Creates an instance of Envoy's Network::ClientConnection. Does NOT initiate the connection;
   * the caller must then call connect() on the returned Network::ClientConnection.
   * @param address supplies the address to connect to.
   * @param source_address supplies an address to bind to or nullptr if no bind is necessary.
   * @param transport_socket supplies a transport socket to be used by the connection.
   * @param options the socket options to be set on the underlying socket before anything is sent
   *        on the socket.
   * @return Network::ClientConnectionPtr a client connection that is owned by the caller.
   */
  ClientConnectionPtr
  createClientConnection(Address::InstanceConstSharedPtr address,
                         Address::InstanceConstSharedPtr source_address,
                         TransportSocketPtr&& transport_socket,
                         const ConnectionSocket::OptionsSharedPtr& options) override;

  /**
   * Register the instance of InternalListenerManager. The internal manager is used to locate the
   * target listener when creating the connection to the envoy internal address. The internal
   * listener manager can be registered only once.
   */
  void registerInternalListenerManager(
      Network::InternalListenerManager& internal_listener_manager) override;

private:
  Network::InternalListenerManagerOptRef internal_listener_manager_;
  Event::Dispatcher& dispatcher_;
};

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