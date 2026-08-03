#include "envoy/extensions/filters/network/downstream_watermark_stats/v3/downstream_watermark_stats.pb.h"

#include "source/common/stats/isolated_store_impl.h"
#include "source/extensions/filters/network/downstream_watermark_stats/downstream_watermark_stats.h"

#include "test/mocks/network/mocks.h"
#include "test/test_common/utility.h"

#include "gtest/gtest.h"

using testing::NiceMock;

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DownstreamWatermarkStats {
namespace {

class DownstreamWatermarkStatsFilterTest : public testing::Test {
public:
  void initialize() {
    envoy::extensions::filters::network::downstream_watermark_stats::v3::DownstreamWatermarkStats
        proto_config;
    proto_config.set_stat_prefix("test_prefix");
    config_ = std::make_shared<Config>(proto_config, *stats_store_.rootScope());
    filter_ = std::make_unique<Filter>(config_);
    filter_->initializeReadFilterCallbacks(read_filter_callbacks_);
  }

  Stats::Gauge& gauge() {
    return *TestUtility::findGauge(stats_store_, "test_prefix.downstream_cx_above_high_watermark");
  }

  Stats::IsolatedStoreImpl stats_store_;
  NiceMock<Network::MockReadFilterCallbacks> read_filter_callbacks_;
  ConfigSharedPtr config_;
  std::unique_ptr<Filter> filter_;
};

TEST_F(DownstreamWatermarkStatsFilterTest, TracksWatermarkState) {
  initialize();
  EXPECT_EQ(0, gauge().value());

  read_filter_callbacks_.connection_.runHighWatermarkCallbacks();
  EXPECT_EQ(1, gauge().value());

  read_filter_callbacks_.connection_.runHighWatermarkCallbacks();
  EXPECT_EQ(1, gauge().value());

  read_filter_callbacks_.connection_.runLowWatermarkCallbacks();
  EXPECT_EQ(0, gauge().value());

  read_filter_callbacks_.connection_.runLowWatermarkCallbacks();
  EXPECT_EQ(0, gauge().value());
}

TEST_F(DownstreamWatermarkStatsFilterTest, ClearsGaugeOnConnectionEvent) {
  initialize();
  read_filter_callbacks_.connection_.runHighWatermarkCallbacks();
  EXPECT_EQ(1, gauge().value());

  filter_->onEvent(Network::ConnectionEvent::RemoteClose);
  EXPECT_EQ(0, gauge().value());
}

TEST_F(DownstreamWatermarkStatsFilterTest, ClearsGaugeOnDestroy) {
  initialize();
  read_filter_callbacks_.connection_.runHighWatermarkCallbacks();
  EXPECT_EQ(1, gauge().value());

  filter_.reset();
  EXPECT_EQ(0, gauge().value());
}

} // namespace
} // namespace DownstreamWatermarkStats
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
