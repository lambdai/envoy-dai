#include <memory>

#include "envoy/api/v2/lds.pb.h"

#include "common/common/logger.h"
#include "common/config/filter_json.h"
#include "common/protobuf/utility.h"

#include "server/lds_api.h"

#include "extensions/filters/network/well_known_names.h"

#include "test/mocks/config/mocks.h"
#include "test/mocks/protobuf/mocks.h"
#include "test/mocks/server/mocks.h"
#include "test/server/utility.h"
#include "test/test_common/environment.h"
#include "test/test_common/utility.h"

#include "benchmark/benchmark.h"
#include "gmock/gmock.h"

using testing::_;
using testing::InSequence;
using testing::Invoke;
using testing::Return;
using testing::ReturnRef;
using testing::Throw;

namespace Envoy {
namespace Server {

class LdsBenchmarkFixture : public benchmark::Fixture {
public:
  using HcmConfigMessage =
      envoy::config::filter::network::http_connection_manager::v2::HttpConnectionManager;
  void SetUp(const ::benchmark::State&) override {
    auto listener_message = parseListenerFromV2Yaml(yaml_);
    auto hcm_message = listener_message.filter_chains(0).filters(0);
    ProtobufWkt::Struct hcm_config_struct = hcm_message.config();

    const Json::ObjectSharedPtr json_config =
        MessageUtil::getJsonObjectFromMessage(hcm_config_struct);
    ENVOY_LOG_MISC(debug, "  config: {}", json_config->asJsonString());

    HcmConfigMessage hcm_config;

    filter_message_struct_ = hcm_message;

    MessageUtil::jsonConvert(hcm_config_struct, ProtobufMessage::getNullValidationVisitor(),
                             hcm_config);
    ProtobufWkt::Any any_message;
    any_message.PackFrom(hcm_config);
    filter_message_any_ = hcm_message;
    *filter_message_any_.mutable_typed_config() = any_message;
    factory_ = &Config::Utility::getAndCheckFactory<Configuration::NamedNetworkFilterConfigFactory>(
        hcm_message.name());
  }

  envoy::api::v2::listener::Filter filter_message_struct_;
  envoy::api::v2::listener::Filter filter_message_any_;
  Envoy::Server::Configuration::NamedNetworkFilterConfigFactory* factory_;
  const std::string yaml_ = TestEnvironment::substitute(R"EOF(
    address:
      socket_address: { address: 127.0.0.1, port_value: 1234 }
    metadata: { filter_metadata: { com.bar.foo: { baz: test_value } } }
    filter_chains:
    - filter_chain_match:
      filters:
      - name: envoy.http_connection_manager
        config:
          codec_type: auto
          stat_prefix: ingress_http
          route_config:
            virtual_hosts:
            - name: backend
              domains:
                - "one.example.com"
                - "www.one.example.com"
              routes:
              - match:
                  prefix: "/"
                route:
                  cluster: targetCluster
          filters:
          - name: envoy.router
            config: 
  )EOF");
};

BENCHMARK_DEFINE_F(LdsBenchmarkFixture, TestStruct)
(::benchmark::State& state) {
  for (auto _ : state) {
    for (int i = 0; i < state.range(0); i++) {
      Config::Utility::translateToFactoryConfig(
          filter_message_struct_, ProtobufMessage::getNullValidationVisitor(), *factory_);
    }
  }
}

BENCHMARK_DEFINE_F(LdsBenchmarkFixture, TestAny)
(::benchmark::State& state) {
  for (auto _ : state) {
    for (int i = 0; i < state.range(0); i++) {
      Config::Utility::translateToFactoryConfig(
          filter_message_any_, ProtobufMessage::getNullValidationVisitor(), *factory_);
    }
  }
}
BENCHMARK_REGISTER_F(LdsBenchmarkFixture, TestStruct)
    ->Ranges({

        {1, 4096},
    });
BENCHMARK_REGISTER_F(LdsBenchmarkFixture, TestAny)
    ->Ranges({
        {1, 4096},
    });

} // namespace Server
} // namespace Envoy
BENCHMARK_MAIN();

/*
opt
------------------------------------------------------------------------------
Benchmark                                    Time             CPU   Iterations
------------------------------------------------------------------------------
LdsBenchmarkFixture/TestStruct/1        135708 ns       135695 ns         5066
LdsBenchmarkFixture/TestStruct/8       1085932 ns      1085826 ns          633
LdsBenchmarkFixture/TestStruct/64      8698649 ns      8697656 ns           81
LdsBenchmarkFixture/TestStruct/512    69195938 ns     69190686 ns           10
LdsBenchmarkFixture/TestStruct/4096  551738739 ns    551696970 ns            1
LdsBenchmarkFixture/TestAny/1             1397 ns         1397 ns       500058
LdsBenchmarkFixture/TestAny/8            11189 ns        11188 ns        62539
LdsBenchmarkFixture/TestAny/64           89325 ns        89316 ns         7857
LdsBenchmarkFixture/TestAny/512         712695 ns       712639 ns          985
LdsBenchmarkFixture/TestAny/4096       5700993 ns      5700542 ns          123
 */