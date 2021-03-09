#include "extensions/io_socket/user_space/connection_factory.h"

#include "common/network/connection_factory.h"

namespace Envoy {
namespace Extensions {
namespace IoSocket {

namespace UserSpace {
Network::ClientConnectionPtr ClientConnectionFactoryImpl::createClientConnection(
    Event::Dispatcher& dispatcher, Network::Address::InstanceConstSharedPtr address,
    Network::Address::InstanceConstSharedPtr source_address,
    Network::TransportSocketPtr transport_socket,
    const Network::ConnectionSocket::OptionsSharedPtr& options) {
  UNREFERENCED_PARAMETER(dispatcher);
  UNREFERENCED_PARAMETER(address);
  UNREFERENCED_PARAMETER(source_address);
  UNREFERENCED_PARAMETER(transport_socket);
  UNREFERENCED_PARAMETER(options);
  return nullptr;
}
} // namespace UserSpace
} // namespace IoSocket
} // namespace Extensions
} // namespace Envoy