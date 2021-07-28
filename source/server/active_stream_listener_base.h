#pragma once

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>

#include "envoy/common/time.h"
#include "envoy/event/dispatcher.h"
#include "envoy/network/connection.h"
#include "envoy/network/connection_handler.h"
#include "envoy/network/listener.h"
#include "envoy/stream_info/stream_info.h"
#include "envoy/stats/timespan.h"

#include "source/common/common/linked_object.h"
#include "source/server/active_listener_base.h"
#include "source/server/active_tcp_socket.h"

namespace Envoy {
namespace Server {


struct ActiveTcpConnection;

/**
 * Wrapper for a group of active connections which are attached to the same filter chain context.
 */
class ActiveConnections : public Event::DeferredDeletable {
public:
  ActiveConnections(ActiveStreamListenerBase& listener, const Network::FilterChain& filter_chain);
  ~ActiveConnections() override;

  // listener filter chain pair is the owner of the connections
  ActiveStreamListenerBase& listener_;
  const Network::FilterChain& filter_chain_;
  // Owned connections
  std::list<std::unique_ptr<ActiveTcpConnection>> connections_;
};

/**
 * Wrapper for an active TCP connection owned by this handler.
 */
struct ActiveTcpConnection : LinkedObject<ActiveTcpConnection>,
                             public Event::DeferredDeletable,
                             public Network::ConnectionCallbacks,
                             Logger::Loggable<Logger::Id::conn_handler> {
  ActiveTcpConnection(ActiveConnections& active_connections,
                      Network::ConnectionPtr&& new_connection, TimeSource& time_system,
                      std::unique_ptr<StreamInfo::StreamInfo>&& stream_info);
  ~ActiveTcpConnection() override;
  using CollectionType = ActiveConnections;
  // Network::ConnectionCallbacks
  void onEvent(Network::ConnectionEvent event) override;
  void onAboveWriteBufferHighWatermark() override {}
  void onBelowWriteBufferLowWatermark() override {}

  std::unique_ptr<StreamInfo::StreamInfo> stream_info_;
  ActiveConnections& active_connections_;
  Network::ConnectionPtr connection_;
  Stats::TimespanPtr conn_length_;
};

// The base class of the stream listener. It owns listener filter handling of active sockets.
// After the active socket passes all the listener filters, a server connection is created. The
// derived listener must override ``newActiveConnection`` to take the ownership of that server
// connection.
class ActiveStreamListenerBase : public ActiveListenerImplBase,
                                 protected Logger::Loggable<Logger::Id::conn_handler> {
public:
  ActiveStreamListenerBase(Network::ConnectionHandler& parent, Event::Dispatcher& dispatcher,
                           Network::ListenerPtr&& listener, Network::ListenerConfig& config);
  static void emitLogs(Network::ListenerConfig& config, StreamInfo::StreamInfo& stream_info);

  Event::Dispatcher& dispatcher() { return dispatcher_; }

  /**
   * Schedule to remove and destroy the active connections which are not tracked by listener
   * config. Caution: The connection are not destroyed yet when function returns.
   */
  void
  deferredRemoveFilterChains(const std::list<const Network::FilterChain*>& draining_filter_chains) {
    // Need to recover the original deleting state.
    const bool was_deleting = is_deleting_;
    is_deleting_ = true;
    for (const auto* filter_chain : draining_filter_chains) {
      removeFilterChain(filter_chain);
    }
    is_deleting_ = was_deleting;
  }

  virtual void incNumConnections() PURE;
  virtual void decNumConnections() PURE;

  /**
   * Create a new connection from a socket accepted by the listener.
   */
  void newConnection(Network::ConnectionSocketPtr&& socket,
                     std::unique_ptr<StreamInfo::StreamInfo> stream_info);

  /**
   * Remove the socket from this listener. Should be called when the socket passes the listener
   * filter.
   * @return std::unique_ptr<ActiveTcpSocket> the exact same socket in the parameter but in the
   * state that not owned by the listener.
   */
  std::unique_ptr<ActiveTcpSocket> removeSocket(ActiveTcpSocket&& socket) {
    return socket.removeFromList(sockets_);
  }

  /**
   * @return const std::list<std::unique_ptr<ActiveTcpSocket>>& the sockets going through the
   * listener filters.
   */
  const std::list<std::unique_ptr<ActiveTcpSocket>>& sockets() const { return sockets_; }


  virtual Network::BalancedConnectionHandlerOptRef
  getBalancedHandlerByAddress(const Network::Address::Instance& address) PURE;

  void onSocketAccepted(std::unique_ptr<ActiveTcpSocket> active_socket) {
    // Create and run the filters
    config_->filterChainFactory().createListenerFilterChain(*active_socket);
    active_socket->continueFilterChain(true);

    // Move active_socket to the sockets_ list if filter iteration needs to continue later.
    // Otherwise we let active_socket be destructed when it goes out of scope.
    if (active_socket->iter_ != active_socket->accept_filters_.end()) {
      active_socket->startTimer();
      LinkedList::moveIntoListBack(std::move(active_socket), sockets_);
    } else {
      if (!active_socket->connected_) {
        // If active_socket is about to be destructed, emit logs if a connection is not created.
        if (active_socket->stream_info_ != nullptr) {
          emitLogs(*config_, *active_socket->stream_info_);
        } else {
          // If the active_socket is not connected, this socket is not promoted to active
          // connection. Thus the stream_info_ is owned by this active socket.
          ENVOY_BUG(active_socket->stream_info_ != nullptr,
                    "the unconnected active socket must have stream info.");
        }
      }
    }
  }

  // Below members are open to access by ActiveTcpSocket.
  Network::ConnectionHandler& parent_;
  const std::chrono::milliseconds listener_filters_timeout_;
  const bool continue_on_listener_filters_timeout_;

protected:
  /**
   * Create the active connection from server connection. This active listener owns the created
   * active connection.
   *
   * @param filter_chain The network filter chain linking to the connection.
   * @param server_conn_ptr The server connection.
   * @param stream_info The stream info of the active connection.
   */
  virtual void newActiveConnection(const Network::FilterChain& filter_chain,
                                   Network::ServerConnectionPtr server_conn_ptr,
                                   std::unique_ptr<StreamInfo::StreamInfo> stream_info) PURE;

  std::list<std::unique_ptr<ActiveTcpSocket>> sockets_;
  Network::ListenerPtr listener_;
  // True if the follow up connection deletion is raised by the connection collection deletion is
  // performing. Otherwise, the collection should be deleted when the last connection in the
  // collection is removed. This state is maintained in base class because this state is independent
  // from concrete connection type.
  bool is_deleting_{false};

private:
  Event::Dispatcher& dispatcher_;
// };

// // The listener that handles the composition type ActiveConnectionCollection. This mixin-ish class
// // provides the connection removal helper and the filter chain removal helper. Meanwhile, the
// // derived class can use the concrete active connection type.
// template <typename ActiveConnectionType>
// class TypedActiveStreamListenerBase : public ActiveStreamListenerBase {
// public:
//   using ActiveConnectionCollectionType = typename ActiveConnectionType::CollectionType;
//   TypedActiveStreamListenerBase(Network::ConnectionHandler& parent, Event::Dispatcher& dispatcher,
//                                 Network::ListenerPtr&& listener, Network::ListenerConfig& config)
//       : ActiveStreamListenerBase(parent, dispatcher, std::move(listener), config) {}
//   using ActiveConnectionPtr = std::unique_ptr<ActiveConnectionType>;
//   using ActiveConnectionCollectionPtr = std::unique_ptr<ActiveConnectionCollectionType>;

using ActiveConnectionPtr = std::unique_ptr<ActiveTcpConnection>;
using ActiveConnectionCollectionPtr = std::unique_ptr<ActiveConnections>;

public:
  /**
   * Remove and destroy an active connection.
   * @param connection supplies the connection to remove.
   */
  void removeConnection(ActiveTcpConnection& connection) {
    ENVOY_CONN_LOG(debug, "adding to cleanup list", *connection.connection_);
    ActiveConnections& active_connections = connection.active_connections_;
    ActiveConnectionPtr removed = connection.removeFromList(active_connections.connections_);
    dispatcher().deferredDelete(std::move(removed));
    // Delete map entry only iff connections becomes empty.
    if (active_connections.connections_.empty()) {
      auto iter = connections_by_context_.find(&active_connections.filter_chain_);
      ASSERT(iter != connections_by_context_.end());
      // To cover the lifetime of every single connection, Connections need to be deferred deleted
      // because the previously contained connection is deferred deleted.
      dispatcher().deferredDelete(std::move(iter->second));
      // The erase will break the iteration over the connections_by_context_ during the deletion.
      if (!is_deleting_) {
        connections_by_context_.erase(iter);
      }
    }
  }

protected:
  /**
   * Schedule removal and destruction of all active connections owned by a filter chain.
   */
  void removeFilterChain(const Network::FilterChain* filter_chain) {
    auto iter = connections_by_context_.find(filter_chain);
    if (iter == connections_by_context_.end()) {
      // It is possible when listener is stopping.
    } else {
      auto& connections = iter->second->connections_;
      while (!connections.empty()) {
        connections.front()->connection_->close(Network::ConnectionCloseType::NoFlush);
      }
      // Since is_deleting_ is on, we need to manually remove the map value and drive the
      // iterator. Defer delete connection container to avoid race condition in destroying
      // connection.
      dispatcher().deferredDelete(std::move(iter->second));
      connections_by_context_.erase(iter);
    }
  }

  absl::flat_hash_map<const Network::FilterChain*, ActiveConnectionCollectionPtr>
      connections_by_context_;
};

} // namespace Server
} // namespace Envoy
