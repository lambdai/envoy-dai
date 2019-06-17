#pragma once

#include <memory>

#include "envoy/api/v2/listener/listener.pb.h"
#include "envoy/protobuf/message_validator.h"
#include "envoy/server/transport_socket_config.h"

#include "common/common/logger.h"
#include "common/network/cidr_range.h"
#include "common/network/lc_trie.h"

#include "server/fcds_api.h"
#include "server/lds_api.h"

#include "absl/container/flat_hash_map.h"

namespace Envoy {
namespace Server {

class FilterChainFactoryBuilder {
public:
  virtual ~FilterChainFactoryBuilder() = default;
  virtual std::unique_ptr<Network::FilterChain>
  buildFilterChain(const ::envoy::api::v2::listener::FilterChain& filter_chain) const = 0;
};

/**
 * Implementation of FilterChainManager.
 * Encapulating the subscription from FCDS Api. The view from Listener is always 
 */
class FilterChainManagerImpl : public Network::FilterChainManager,
                               public Config::SubscriptionCallbacks,
                               Logger::Loggable<Logger::Id::config> {
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

  void
  addFilterChain(absl::Span<const ::envoy::api::v2::listener::FilterChainConfiguration* const> filter_chain_names,
                 FilterChainFactoryBuilder& b) {}

  static bool isWildcardServerName(const std::string& name);

  // TODO(silentdai): implement Config::SubscriptionCallbacks
  void onConfigUpdate(const Protobuf::RepeatedPtrField<ProtobufWkt::Any>& resources,
                      const std::string& version_info) override {}                 
  void onConfigUpdate(const Protobuf::RepeatedPtrField<envoy::api::v2::Resource>& added_resources,
                      const Protobuf::RepeatedPtrField<std::string>& removed_resources,
                      const std::string& system_version_info) {
                      }
  void onConfigUpdateFailed(const EnvoyException* e) {}

  struct FilterChainLookup {
    DestinationPortsMap destination_ports_map_;
    // LocalInitManager or server_global_init_manager;
    void init_done() {
      /*
      filter_chain_manager.signal("this lookup is warmed", this)
      if (belonging listener is not ready) {
        signal listener;
        // if listener attempted to create more than one lookup, it's listener that waiting for the last ready.
      } else {
        signal fc manager that filter_chain_Lookup is ready. This may requires something other than
        init manager where ready is only available at NOT_YET_READY state.
      }
       */ 
    }
  };
private:
  // Stateless method. Will optimize in the future.
  // This is the new filter chains. It could originates from FCDS or LDS.
  void resetFilterChains(absl::Span<const envoy::api::v2::listener::FilterChain> filter_chains) {}

  void
  addFilterChainInternal(absl::Span<const ::envoy::api::v2::listener::FilterChain* const> filter_chain_span,
                 FilterChainFactoryBuilder& b);
  void convertIPsToTries();
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
  DestinationPortsMap destination_ports_map_;
  std::shared_ptr<FilterChainLookup> active_lookup_;
  std::shared_ptr<FilterChainLookup> warming_lookup_;

  Network::Address::InstanceConstSharedPtr address_;
  ProtobufMessage::ValidationVisitor& validation_visitor_;
  // FilterChainList active_filter_chains_;
  // FilterChainList warming_filter_chains_;
  // std::list<DrainingFilterChain> draining_filter_chain_;
  // ConfigTracker::EntryOwnerPtr config_tracker_entry_;
  // FcdsApiPtr fcds_api_;
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