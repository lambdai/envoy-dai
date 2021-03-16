#include "common/network/connection_factory_impl.h"

namespace Envoy {
namespace Network {

ServerConnectionPtr
ConnectionFactoryImpl::createServerConnection(ConnectionSocketPtr&& socket,
                                              TransportSocketPtr&& transport_socket,
                                              StreamInfo::StreamInfo& stream_info) {
  ASSERT(dispatcher_.isThreadSafe());
  return std::make_unique<Network::ServerConnectionImpl>(
      dispatcher_, std::move(socket), std::move(transport_socket), stream_info, true);
}

ClientConnectionPtr ConnectionFactoryImpl::createClientConnection(
    Address::InstanceConstSharedPtr address, Address::InstanceConstSharedPtr source_address,
    TransportSocketPtr&& transport_socket, const ConnectionSocket::OptionsSharedPtr& options) {
  ASSERT(dispatcher_.isThreadSafe());
  auto* client_connection_factory = address->clientConnectionFactory();
  if (client_connection_factory) {
    return client_connection_factory->createClientConnection(dispatcher_, address, source_address,
                                                             std::move(transport_socket), options);
  }
  return std::make_unique<Network::ClientConnectionImpl>(dispatcher_, address, source_address,
                                                         std::move(transport_socket), options);
}

void ConnectionFactoryImpl::registerInternalListenerManager(
    Network::InternalListenerManager& internal_listener_manager) {
  ASSERT(dispatcher_.isThreadSafe());
  ASSERT(!internal_listener_manager_,
         "Each dispatcher can have at most one registered internal listener manager.");
  internal_listener_manager_ = internal_listener_manager;
}

} // namespace Network
} // namespace Envoy