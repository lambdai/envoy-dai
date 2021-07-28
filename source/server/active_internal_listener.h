#pragma once
#include <atomic>
#include <cstdint>
#include <list>
#include <memory>

#include "envoy/common/time.h"
#include "envoy/event/deferred_deletable.h"
#include "envoy/event/dispatcher.h"
#include "envoy/network/connection.h"
#include "envoy/network/connection_handler.h"
#include "envoy/network/filter.h"
#include "envoy/network/listen_socket.h"
#include "envoy/network/listener.h"
#include "envoy/server/listener_manager.h"
#include "envoy/stats/scope.h"
#include "envoy/stats/timespan.h"

#include "source/common/common/linked_object.h"
#include "source/common/common/non_copyable.h"
#include "source/common/stream_info/stream_info_impl.h"
#include "source/server/active_tcp_socket.h"
#include "source/server/active_stream_listener_base.h"

#include "spdlog/spdlog.h"

namespace Envoy {
namespace Server {

using ActiveInternalConnections = ActiveConnections;
using ActiveInternalConnectionPtr = std::unique_ptr<ActiveTcpConnection>;
class ActiveInternalListener :
    // stash(lambdai): remove template
    public ActiveStreamListenerBase,
    public Network::InternalListenerCallbacks {
public:
  ActiveInternalListener(Network::ConnectionHandler& conn_handler, Event::Dispatcher& dispatcher,
                         Network::ListenerConfig& config);
  ActiveInternalListener(Network::ConnectionHandler& conn_handler, Event::Dispatcher& dispatcher,
                         Network::ListenerPtr listener, Network::ListenerConfig& config);
  ~ActiveInternalListener() override;

  class NetworkInternalListener : public Network::Listener {

    void disable() override {
      // TODO(lambdai): think about how to elegantly disable internal listener. (Queue socket or
      // close socket immediately?)
      ENVOY_LOG(debug, "Warning: the internal listener cannot be disabled.");
    }

    void enable() override {
      ENVOY_LOG(debug, "Warning: the internal listener is always enabled.");
    }

    void setRejectFraction(UnitFloat) override {}
  };
  // // ActiveListenerImplBase
  // Network::Listener* listener() override { return listener_.get(); }

  Network::BalancedConnectionHandlerOptRef
  getBalancedHandlerByAddress(const Network::Address::Instance&) override {
    NOT_IMPLEMENTED_GCOVR_EXCL_LINE;
  }

  void pauseListening() override {
    if (listener_ != nullptr) {
      listener_->disable();
    }
  }
  void resumeListening() override {
    if (listener_ != nullptr) {
      listener_->enable();
    }
  }
  void shutdownListener() override { listener_.reset(); }

  // Network::InternalListenerCallbacks
  void onAccept(Network::ConnectionSocketPtr&& socket) override;
  // Event::Dispatcher& dispatcher() override { return dispatcher_; }

  void newActiveConnection(const Network::FilterChain& filter_chain,
                           Network::ServerConnectionPtr server_conn_ptr,
                           std::unique_ptr<StreamInfo::StreamInfo> stream_info) override;

  void incNumConnections() override { config_->openConnections().inc(); }
  void decNumConnections() override { config_->openConnections().dec(); }

  /**
   * Return the active connections container attached with the given filter chain.
   */
  ActiveInternalConnections& getOrCreateActiveConnections(const Network::FilterChain& filter_chain);

  /**
   * Update the listener config. The follow up connections will see the new config. The existing
   * connections are not impacted.
   */
  void updateListenerConfig(Network::ListenerConfig& config);
};

} // namespace Server
} // namespace Envoy
