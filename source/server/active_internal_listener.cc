#include "source/server/active_internal_listener.h"

#include "envoy/network/filter.h"
#include "envoy/stats/scope.h"

#include "source/common/network/address_impl.h"
#include "source/common/stats/timespan_impl.h"

namespace Envoy {
namespace Server {

using ActiveInternalConnection = ActiveTcpConnection;
using ActiveInternalConnectionPtr = std::unique_ptr<ActiveTcpConnection>;
using ActiveInternalConnections = ActiveConnections;
using ActiveInternalConnectionsPtr = std::unique_ptr<ActiveInternalConnections>;

ActiveInternalListener::ActiveInternalListener(Network::ConnectionHandler& conn_handler,
                                               Event::Dispatcher& dispatcher,
                                               Network::ListenerConfig& config)
    : ActiveStreamListenerBase(
          conn_handler, dispatcher,
          std::make_unique<ActiveInternalListener::NetworkInternalListener>(), config) {}
ActiveInternalListener::ActiveInternalListener(Network::ConnectionHandler& conn_handler,
                                               Event::Dispatcher& dispatcher,
                                               Network::ListenerPtr listener,
                                               Network::ListenerConfig& config)
    : ActiveStreamListenerBase(conn_handler, dispatcher,
                                                              std::move(listener), config) {}

ActiveInternalListener::~ActiveInternalListener() {   
  is_deleting_ = true;
  // Purge sockets that have not progressed to connections. This should only happen when
  // a listener filter stops iteration and never resumes.
  while (!sockets_.empty()) {
    auto removed = sockets_.front()->removeFromList(sockets_);
    dispatcher().deferredDelete(std::move(removed));
  }

  for (auto& [chain, active_connections] : connections_by_context_) {
    ASSERT(active_connections != nullptr);
    auto& connections = active_connections->connections_;
    while (!connections.empty()) {
      connections.front()->connection_->close(Network::ConnectionCloseType::NoFlush);
    }
  }
  dispatcher().clearDeferredDeleteList();
 }

void ActiveInternalListener::newActiveConnection(
    const Network::FilterChain& filter_chain, Network::ServerConnectionPtr server_conn_ptr,
    std::unique_ptr<StreamInfo::StreamInfo> stream_info) {
  auto& active_connections = getOrCreateActiveConnections(filter_chain);
  ActiveInternalConnectionPtr active_connection(
      new ActiveInternalConnection(active_connections, std::move(server_conn_ptr),
                                   dispatcher().timeSource(), std::move(stream_info)));

  // If the connection is already closed, we can just let this connection immediately die.
  if (active_connection->connection_->state() != Network::Connection::State::Closed) {
    ENVOY_CONN_LOG(debug, "new connection", *active_connection->connection_);
    active_connection->connection_->addConnectionCallbacks(*active_connection);
    LinkedList::moveIntoList(std::move(active_connection), active_connections.connections_);
  }
}

ActiveInternalConnections&
ActiveInternalListener::getOrCreateActiveConnections(const Network::FilterChain& filter_chain) {
  ActiveInternalConnectionsPtr& connections = connections_by_context_[&filter_chain];
  if (connections == nullptr) {
    connections = std::make_unique<ActiveInternalConnections>(*this, filter_chain);
  }
  return *connections;
}

void ActiveInternalListener::updateListenerConfig(Network::ListenerConfig& config) {
  ENVOY_LOG(trace, "replacing listener ", config_->listenerTag(), " by ", config.listenerTag());
  config_ = &config;
}

// ActiveInternalConnections::ActiveInternalConnections(ActiveInternalListener& listener,
//                                                      const Network::FilterChain& filter_chain)
//     : listener_(listener), filter_chain_(filter_chain) {}

// ActiveInternalConnections::~ActiveInternalConnections() {
//   // connections should be defer deleted already.
//   ASSERT(connections_.empty());
// }

// ActiveInternalConnection::ActiveInternalConnection(
//     ActiveInternalConnections& active_connections, Network::ConnectionPtr&& new_connection,
//     TimeSource& time_source, std::unique_ptr<StreamInfo::StreamInfo>&& stream_info)
//     : stream_info_(std::move(stream_info)), active_connections_(active_connections),
//       connection_(std::move(new_connection)),
//       conn_length_(new Stats::HistogramCompletableTimespanImpl(
//           active_connections_.listener_.stats_.downstream_cx_length_ms_, time_source)) {
//   // We just universally set no delay on connections. Theoretically we might at some point want
//   // to make this configurable.
//   connection_->noDelay(true);
//   auto& listener = active_connections_.listener_;
//   listener.stats_.downstream_cx_total_.inc();
//   listener.stats_.downstream_cx_active_.inc();
//   listener.per_worker_stats_.downstream_cx_total_.inc();
//   listener.per_worker_stats_.downstream_cx_active_.inc();

//   // Active connections on the handler (not listener). The per listener connections have already
//   // been incremented at this point either via the connection balancer or in the socket accept
//   // path if there is no configured balancer.
//   listener.parent_.incNumConnections();
// }

// ActiveInternalConnection::~ActiveInternalConnection() {
//   ActiveInternalListener::emitLogs(*active_connections_.listener_.config_, *stream_info_);
//   auto& listener = active_connections_.listener_;
//   listener.stats_.downstream_cx_active_.dec();
//   listener.stats_.downstream_cx_destroy_.inc();
//   listener.per_worker_stats_.downstream_cx_active_.dec();
//   conn_length_->complete();

//   // Active listener connections (not handler).
//   listener.decNumConnections();

//   // Active handler connections (not listener).
//   listener.parent_.decNumConnections();
// }

void ActiveInternalListener::onAccept(Network::ConnectionSocketPtr&& socket) {
  // Unlike tcp listener, no rebalancer is applied and won't call pickTargetHandler to account
  // connections.
  incNumConnections();

  auto active_socket = std::make_unique<ActiveTcpSocket>(
      *this, std::move(socket), false /* do not handle off at internal listener */);
  // TODO(lambdai): restore address from either socket options, or from listener config.
  active_socket->socket_->addressProvider().restoreLocalAddress(
      std::make_shared<Network::Address::Ipv4Instance>("255.255.255.255", 0));
  active_socket->socket_->addressProvider().setRemoteAddress(
      std::make_shared<Network::Address::Ipv4Instance>("255.255.255.254", 0));

  onSocketAccepted(std::move(active_socket));
}

} // namespace Server
} // namespace Envoy
