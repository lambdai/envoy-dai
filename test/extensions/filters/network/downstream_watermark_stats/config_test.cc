#include "source/common/stats/isolated_store_impl.h"
#include "source/extensions/filters/network/downstream_watermark_stats/config.h"

#include "test/mocks/server/factory_context.h"
#include "test/test_common/status_utility.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::NiceMock;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DownstreamWatermarkStats {
namespace {

class DownstreamWatermarkStatsConfigTest : public testing::Test {
public:
  void setup(absl::string_view yaml) {
    envoy::extensions::filters::network::downstream_watermark_stats::v3::DownstreamWatermarkStats
        proto_config;
    TestUtility::loadFromYamlAndValidate(std::string(yaml), proto_config);

    ON_CALL(context_, scope()).WillByDefault(ReturnRef(*stats_store_.rootScope()));
    auto result = factory_.createFilterFactoryFromProto(proto_config, context_);
    EXPECT_OK(result);
    cb_ = result.value();
  }

  Stats::IsolatedStoreImpl stats_store_;
  NiceMock<Server::Configuration::MockFactoryContext> context_;
  DownstreamWatermarkStatsConfigFactory factory_;
  Network::FilterFactoryCb cb_;
};

TEST_F(DownstreamWatermarkStatsConfigTest, BasicConfig) {
  setup(R"EOF(
stat_prefix: test
)EOF");

  Network::MockFilterManager filter_manager;
  EXPECT_CALL(filter_manager, addReadFilter(_));
  cb_(filter_manager);
}

TEST_F(DownstreamWatermarkStatsConfigTest, FactoryName) {
  EXPECT_EQ("envoy.filters.network.downstream_watermark_stats", factory_.name());
}

} // namespace
} // namespace DownstreamWatermarkStats
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
