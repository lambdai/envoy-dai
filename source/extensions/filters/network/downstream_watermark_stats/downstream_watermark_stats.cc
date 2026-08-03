#include "source/extensions/filters/network/downstream_watermark_stats/downstream_watermark_stats.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DownstreamWatermarkStats {

Config::Config(const envoy::extensions::filters::network::downstream_watermark_stats::v3::
                   DownstreamWatermarkStats& proto_config,
               Stats::Scope& scope)
    : stats_(generateStats(proto_config.stat_prefix(), scope)) {}

FilterStats Config::generateStats(absl::string_view prefix, Stats::Scope& scope) {
  return {ALL_DOWNSTREAM_WATERMARK_STATS(POOL_GAUGE_PREFIX(scope, prefix))};
}

Filter::~Filter() { clearAboveHighWatermark(); }

void Filter::initializeReadFilterCallbacks(Network::ReadFilterCallbacks& read_callbacks) {
  read_callbacks_ = &read_callbacks;
  read_callbacks_->connection().addConnectionCallbacks(*this);
}

void Filter::onEvent(Network::ConnectionEvent) { clearAboveHighWatermark(); }

void Filter::onAboveWriteBufferHighWatermark() {
  if (!above_high_watermark_) {
    above_high_watermark_ = true;
    config_->stats().downstream_cx_above_high_watermark_.inc();
  }
}

void Filter::onBelowWriteBufferLowWatermark() { clearAboveHighWatermark(); }

void Filter::clearAboveHighWatermark() {
  if (above_high_watermark_) {
    above_high_watermark_ = false;
    config_->stats().downstream_cx_above_high_watermark_.dec();
  }
}

} // namespace DownstreamWatermarkStats
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
