#pragma once

#include <memory>

#include "envoy/api/v2/listener/listener.pb.h"
#include "envoy/protobuf/message_validator.h"
#include "envoy/server/transport_socket_config.h"

#include "common/common/logger.h"
#include "common/init/manager_impl.h"
#include "common/network/cidr_range.h"
#include "common/network/lc_trie.h"

#include "server/fcds_api.h"
#include "server/lds_api.h"

#include "absl/container/flat_hash_map.h"

namespace Envoy {
namespace Server {

class FilterChainImpl;

class FilterChainFactoryBuilder {
public:
  virtual ~FilterChainFactoryBuilder() = default;
  virtual std::unique_ptr<Network::FilterChain>
  buildFilterChain(const ::envoy::api::v2::listener::FilterChain& filter_chain) const = 0;
};

/**
 * Implementation of FilterChainManager.
 * Encapulating the subscription from FCDS Api.
 */
class FilterChainManagerImpl : public Network::FilterChainManager,
                               public Config::SubscriptionCallbacks,
                               Logger::Loggable<Logger::Id::config> {

  using FcProto = ::envoy::api::v2::listener::FilterChain;

public:
  FilterChainManagerImpl(Network::Address::InstanceConstSharedPtr address,
                         ProtobufMessage::ValidationVisitor& visitor)
      : address_(address), validation_visitor_(visitor) {}

  // Network::FilterChainManager
  const Network::FilterChain*
  findFilterChain(const Network::ConnectionSocket& socket) const override;

  void
  addFilterChain(absl::Span<const ::envoy::api::v2::listener::FilterChain* const> filter_chain_span,
                 FilterChainFactoryBuilder& b);

  void addFilterChainInternalForFcds(
      absl::Span<const ::envoy::api::v2::listener::FilterChain* const> filter_chain_span,
      FilterChainFactoryBuilder& b);
  void addFilterChainInternal(
      absl::Span<const ::envoy::api::v2::listener::FilterChain* const> filter_chain_span,
      FilterChainFactoryBuilder& b);

  // TODO(silentdai) : future interface. Listener provides only names.
  // void addFilterChain(absl::Span<const ::envoy::api::v2::listener::FilterChainConfiguration*
  // const>
  //                         filter_chain_names,
  //                     FilterChainFactoryBuilder& b) {}

  static bool isWildcardServerName(const std::string& name);

  // TODO(silentdai): implement Config::SubscriptionCallbacks
  void onConfigUpdate(const Protobuf::RepeatedPtrField<ProtobufWkt::Any>& resources,
                      const std::string& version_info) override {
    ENVOY_LOG(info, "{}{}", resources.size(), version_info);
  }
  void onConfigUpdate(const Protobuf::RepeatedPtrField<envoy::api::v2::Resource>& added_resources,
                      const Protobuf::RepeatedPtrField<std::string>& removed_resources,
                      const std::string& system_version_info) override {
    ENVOY_LOG(info, "{}{}{}", added_resources.size(), removed_resources.size(),
              system_version_info);
  }
  void onConfigUpdateFailed(const EnvoyException* e) override { throw *e; }

  std::string resourceName(const ProtobufWkt::Any& resource) override {
    return MessageUtil::anyConvert<envoy::api::v2::listener::FilterChain>(resource,
                                                                          validation_visitor_)
        .name();
  }
  // In order to share between internal class
  using SourcePortsMap = absl::flat_hash_map<uint16_t, Network::FilterChainSharedPtr>;
  using SourcePortsMapSharedPtr = std::shared_ptr<SourcePortsMap>;
  using SourceIPsMap = absl::flat_hash_map<std::string, SourcePortsMapSharedPtr>;
  using SourceIPsTrie = Network::LcTrie::LcTrie<SourcePortsMapSharedPtr>;
  using SourceIPsTriePtr = std::unique_ptr<SourceIPsTrie>;
  using SourceTypesArray = std::array<std::pair<SourceIPsMap, SourceIPsTriePtr>, 3>;
  using ApplicationProtocolsMap = absl::flat_hash_map<std::string, SourceTypesArray>;
  using TransportProtocolsMap = absl::flat_hash_map<std::string, ApplicationProtocolsMap>;
  // Both exact server names and wildcard domains are part of the same map, in which wildcard
  // domains are prefixed with "." (i.e. ".example.com" for "*.example.com") to differentiate
  // between exact and wildcard entries.
  using ServerNamesMap = absl::flat_hash_map<std::string, TransportProtocolsMap>;
  using ServerNamesMapSharedPtr = std::shared_ptr<ServerNamesMap>;
  using DestinationIPsMap = absl::flat_hash_map<std::string, ServerNamesMapSharedPtr>;
  using DestinationIPsTrie = Network::LcTrie::LcTrie<ServerNamesMapSharedPtr>;
  using DestinationIPsTriePtr = std::unique_ptr<DestinationIPsTrie>;
  using DestinationPortsMap =
      absl::flat_hash_map<uint16_t, std::pair<DestinationIPsMap, DestinationIPsTriePtr>>;
  static void convertIPsToTries(DestinationPortsMap& destination_ports_map);

private:
  void addFilterChainForDestinationPorts(
      DestinationPortsMap& destination_ports_map, uint16_t destination_port,
      const std::vector<std::string>& destination_ips, const std::vector<std::string>& server_names,
      const std::string& transport_protocol, const std::vector<std::string>& application_protocols,
      const envoy::api::v2::listener::FilterChainMatch_ConnectionSourceType source_type,
      const std::vector<std::string>& source_ips,
      const Protobuf::RepeatedField<Protobuf::uint32>& source_ports,
      const Network::FilterChainSharedPtr& filter_chain);
  void addFilterChainForDestinationIPs(
      DestinationIPsMap& destination_ips_map, const std::vector<std::string>& destination_ips,
      const std::vector<std::string>& server_names, const std::string& transport_protocol,
      const std::vector<std::string>& application_protocols,
      const envoy::api::v2::listener::FilterChainMatch_ConnectionSourceType source_type,
      const std::vector<std::string>& source_ips,
      const Protobuf::RepeatedField<Protobuf::uint32>& source_ports,
      const Network::FilterChainSharedPtr& filter_chain);
  void addFilterChainForServerNames(
      ServerNamesMapSharedPtr& server_names_map_ptr, const std::vector<std::string>& server_names,
      const std::string& transport_protocol, const std::vector<std::string>& application_protocols,
      const envoy::api::v2::listener::FilterChainMatch_ConnectionSourceType source_type,
      const std::vector<std::string>& source_ips,
      const Protobuf::RepeatedField<Protobuf::uint32>& source_ports,
      const Network::FilterChainSharedPtr& filter_chain);
  void addFilterChainForApplicationProtocols(
      ApplicationProtocolsMap& application_protocol_map,
      const std::vector<std::string>& application_protocols,
      const envoy::api::v2::listener::FilterChainMatch_ConnectionSourceType source_type,
      const std::vector<std::string>& source_ips,
      const Protobuf::RepeatedField<Protobuf::uint32>& source_ports,
      const Network::FilterChainSharedPtr& filter_chain);
  void addFilterChainForSourceTypes(
      SourceTypesArray& source_types_array,
      const envoy::api::v2::listener::FilterChainMatch_ConnectionSourceType source_type,
      const std::vector<std::string>& source_ips,
      const Protobuf::RepeatedField<Protobuf::uint32>& source_ports,
      const Network::FilterChainSharedPtr& filter_chain);
  void addFilterChainForSourceIPs(SourceIPsMap& source_ips_map, const std::string& source_ip,
                                  const Protobuf::RepeatedField<Protobuf::uint32>& source_ports,
                                  const Network::FilterChainSharedPtr& filter_chain);
  void addFilterChainForSourcePorts(SourcePortsMapSharedPtr& source_ports_map_ptr,
                                    uint32_t source_port,
                                    const Network::FilterChainSharedPtr& filter_chain);

  const Network::FilterChain*
  findFilterChainForDestinationIP(const DestinationIPsTrie& destination_ips_trie,
                                  const Network::ConnectionSocket& socket) const;
  const Network::FilterChain*
  findFilterChainForServerName(const ServerNamesMap& server_names_map,
                               const Network::ConnectionSocket& socket) const;
  const Network::FilterChain*
  findFilterChainForTransportProtocol(const TransportProtocolsMap& transport_protocols_map,
                                      const Network::ConnectionSocket& socket) const;
  const Network::FilterChain*
  findFilterChainForApplicationProtocols(const ApplicationProtocolsMap& application_protocols_map,
                                         const Network::ConnectionSocket& socket) const;
  const Network::FilterChain*
  findFilterChainForSourceTypes(const SourceTypesArray& source_types,
                                const Network::ConnectionSocket& socket) const;

  const Network::FilterChain*
  findFilterChainForSourceIpAndPort(const SourceIPsTrie& source_ips_trie,
                                    const Network::ConnectionSocket& socket) const;

  // Mapping of FilterChain's configured destination ports, IPs, server names, transport protocols
  // and application protocols, using structures defined above.
  struct FilterChainLookup {

    // The indexed filter chain lookup table.
    DestinationPortsMap destination_ports_map_;

    std::unordered_map<FcProto, std::shared_ptr<Network::FilterChain>, MessageUtil, MessageUtil>
        existing_active_filter_chains_;

    // Used during warm up, notified by dependencies, and notify parent that the index is ready.
    // TODO(silentdai): verify that ainitmanager could be released during the release.
    // If not extra logic might be needed to take over the ownership
    // of existing init_manager.
    Init::ManagerImpl dynamic_init_manager_{"filter_chain_init_manager"};

    // `has_active_lookup_` is tricky here. It seems the condition is mutable. However, if we
    // maintain the invariable below, in each lifetime of `FilterChainLookup` the value never
    // change. To `FilterChainLookup` user Only transform the current `warming_lookup_` to
    // `active_lookup_` Access `has_active_lookup_` at the beginning of `FilterChainLookup`. The
    // value is stale soon after. Consider the case
    //   another FCDS update request comes and new warming lookup is created.
    // Access `has_active_lookup_` from main thread.
    bool has_active_lookup_{false};
    // TODO: provide a real callback
    std::function<Init::Manager&()> init_manager_callback_;
    // TODO: replace this
    std::function<void()> warmed_callback_;

    std::unique_ptr<Init::Watcher> init_watcher_;

    Init::Manager& getInitManager() {
      if (has_active_lookup_) {
        return dynamic_init_manager_;
      } else {
        return init_manager_callback_();
      }
    }
    void initialize() {
      if (has_active_lookup_) {
        ENVOY_LOG(info, "initialize lookup with its local init manager");
        dynamic_init_manager_.initialize(*init_watcher_);
      } else {
        // TODO: replace the init manager in factory_context to build the filter chains
        ENVOY_LOG(info, "NOT DONE: replace the init manager when adding filter chain");
        // TODO: tree-init
        // Mark warmed immediately: lookup is ready while dependencies are not. That is
        // ENVOY_LOG(info, "initial lookup active {} : mark {} as active immediately.",
        //           static_cast<void*>(active_lookup_), static_cast<void*>(warming_lookup_));
        warmed_callback_();
      }
    }
  };

  std::unique_ptr<FilterChainLookup> createFilterChainLookup() {
    auto res = std::make_unique<FilterChainLookup>();
    res->has_active_lookup_ = active_lookup_.get() != nullptr;
    ENVOY_LOG(info, "new filter chain lookup has_active_lookup = {}", res->has_active_lookup_);
    // TODO: Maybe it should be evaluate at this point.
    res->init_manager_callback_ = nullptr;
    res->init_watcher_ =
        std::make_unique<Init::WatcherImpl>("filterlookup", [lookup = res.get(), this]() {
          warmed(lookup);
          // TODO tree-init: alternative init manager IMPL
          if (!lookup->has_active_lookup_) {
          }
        });
    res->warmed_callback_ = [lookup = res.get(), this]() {
      ENVOY_LOG(info, "initial lookup active {} warming {} : mark {} as active immediately.",
                 static_cast<void*>(active_lookup_.get()),
                 static_cast<void*>(warming_lookup_.get()),
                 static_cast<void*>(this));
      warmed(lookup);
    };
    return res;
  }

  void warmed(FilterChainLookup* warming_lookup) {
    if (warming_lookup != warming_lookup_.get()) {
      ENVOY_LOG(error, "transforming warmed up lookup {} but is replaced by newer warming one {}. ",
                static_cast<void*>(warming_lookup), static_cast<void*>(warming_lookup_.get()));
      return;
    } else {
      ENVOY_LOG(info, "updating to warmed up lookup {}", static_cast<void*>(warming_lookup));
      std::swap(warming_lookup_, active_lookup_);
    }
  }

  // The invariant:
  // Once the active one is ready, there is always an active one until shutdown.
  // Warming one could be replaced by another warming lookup. The user is responsible for not
  // overriding a new warming one by elder one. The warming lookup may replace the active_lookup
  // atomically, or replaced by another warming up. If warming one is replaced by another warming
  // one, the former one should have no side effect. If the warming eventually replaces the active
  // one, the warming one could assume the active one never changed durign the warm up.
  std::shared_ptr<FilterChainLookup> active_lookup_;
  std::shared_ptr<FilterChainLookup> warming_lookup_;

  // The address should never change during the lifetime of the filter chain manager.
  // TODO(silentdai): declare const
  Network::Address::InstanceConstSharedPtr address_;
  ProtobufMessage::ValidationVisitor& validation_visitor_;
};

class FilterChainImpl : public Network::FilterChain {
public:
  FilterChainImpl(Network::TransportSocketFactoryPtr&& transport_socket_factory,
                  std::vector<Network::FilterFactoryCb>&& filters_factory)
      : transport_socket_factory_(std::move(transport_socket_factory)),
        filters_factory_(std::move(filters_factory)) {}

  // Network::FilterChain
  const Network::TransportSocketFactory& transportSocketFactory() const override {
    return *transport_socket_factory_;
  }

  const std::vector<Network::FilterFactoryCb>& networkFilterFactories() const override {
    return filters_factory_;
  }

private:
  const Network::TransportSocketFactoryPtr transport_socket_factory_;
  const std::vector<Network::FilterFactoryCb> filters_factory_;
};

} // namespace Server
} // namespace Envoy