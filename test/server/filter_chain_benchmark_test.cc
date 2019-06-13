#include <iostream>
#include <unordered_map>

#include "envoy/protobuf/message_validator.h"

#include "server/filter_chain_manager_impl.h"

#include "extensions/transport_sockets/well_known_names.h"

#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "benchmark/benchmark.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Server {

namespace {

class MockFilterChainFactoryBuilder : public FilterChainFactoryBuilder {
  std::unique_ptr<Network::FilterChain>
  buildFilterChain(const ::envoy::api::v2::listener::FilterChain&) const override {
    return nullptr;
  }
};

const std::string yaml_header = R"EOF(
    address:
      socket_address: { address: 127.0.0.1, port_value: 1234 }
    listener_filters:
    - name: "envoy.listener.tls_inspector"
      config: {}
    filter_chains:
    - filter_chain_match:
        # empty
      tls_context:
        common_tls_context:
          tls_certificates:
            - certificate_chain: { filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/san_uri_cert.pem" }
              private_key: { filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/san_uri_key.pem" }
        session_ticket_keys:
          keys:
          - filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/ticket_key_a")EOF";
const std::string yaml_single_server = R"EOF(
    - filter_chain_match:
        server_names: "server1.example.com"
        transport_protocol: "tls"
      tls_context:
        common_tls_context:
          tls_certificates:
            - certificate_chain: { filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/san_dns_cert.pem" }
              private_key: { filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/san_dns_key.pem" }
        session_ticket_keys:
          keys:
          - filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/ticket_key_a")EOF";
const std::string yaml_single_dst_port_top = R"EOF(
    - filter_chain_match:
        destination_port: )EOF";
const std::string yaml_single_dst_port_bottom = R"EOF(
      tls_context:
        common_tls_context:
          tls_certificates:
            - certificate_chain: { filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/san_multiple_dns_cert.pem" }
              private_key: { filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/san_multiple_dns_key.pem" }
        session_ticket_keys:
          keys:
          - filename: "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/ticket_key_a")EOF";
} // namespace

class FilterChainBenchmarkFixture : public benchmark::Fixture {
public:
  void SetUp(const ::benchmark::State& state) {
    int64_t input_size = state.range(0);
    std::vector<std::string> port_chains;
    for (int i = 0; i < input_size; i++) {
      port_chains.push_back(
          absl::StrCat(yaml_single_dst_port_top, 10000 + i, yaml_single_dst_port_bottom));
    }
    listener_yaml_config_ = TestEnvironment::substitute(
        absl::StrCat(yaml_header, yaml_single_server, absl::StrJoin(port_chains, "")),
        Network::Address::IpVersion::v4);
    TestUtility::loadFromYaml(listener_yaml_config_, listener_config_);
    filter_chains_ = listener_config_.filter_chains();
  }
  absl::Span<const envoy::api::v2::listener::FilterChain* const> filter_chains_;
  std::string listener_yaml_config_;
  envoy::api::v2::Listener listener_config_;
  MockFilterChainFactoryBuilder dummy_builder_;
  FilterChainManagerImpl filter_chain_manager_{
      std::make_shared<Network::Address::Ipv4Instance>("127.0.0.1", 1234),
      ProtobufMessage::getNullValidationVisitor()};
};

BENCHMARK_DEFINE_F(FilterChainBenchmarkFixture, ListenerConfigUpdateTest)
(::benchmark::State& state) {
  for (auto _ : state) {
    FilterChainManagerImpl filter_chain_manager{
        std::make_shared<Network::Address::Ipv4Instance>("127.0.0.1", 1234),
        ProtobufMessage::getNullValidationVisitor()};
    filter_chain_manager.addFilterChain(filter_chains_, dummy_builder_);
  }
}
BENCHMARK_REGISTER_F(FilterChainBenchmarkFixture, ListenerConfigUpdateTest)
    ->Ranges({
        // scale of the chains
        {1, 64},
    });

/*
--------------------------------------------------------------------------------------------------
Benchmark                                                        Time             CPU   Iterations
--------------------------------------------------------------------------------------------------
FilterChainBenchmarkFixture/ListenerConfigUpdateTest/1     2605268 ns      2604634 ns          267
FilterChainBenchmarkFixture/ListenerConfigUpdateTest/8     8674554 ns      8666495 ns           81
FilterChainBenchmarkFixture/ListenerConfigUpdateTest/64   57806353 ns     57717707 ns           12
*/
} // namespace Server
} // namespace Envoy
BENCHMARK_MAIN();
