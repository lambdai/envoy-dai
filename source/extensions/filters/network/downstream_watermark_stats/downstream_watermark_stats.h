#pragma once

#include "envoy/extensions/filters/network/downstream_watermark_stats/v3/downstream_watermark_stats.pb.h"
#include "envoy/network/connection.h"
#include "envoy/network/filter.h"
#include "envoy/stats/stats_macros.h"

#include "source/common/common/logger.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DownstreamWatermarkStats {

#define ALL_DOWNSTREAM_WATERMARK_STATS(GAUGE) GAUGE(downstream_cx_above_high_watermark, Accumulate)

struct FilterStats {
  ALL_DOWNSTREAM_WATERMARK_STATS(GENERATE_GAUGE_STRUCT)
};

class Config {
public:
  Config(const envoy::extensions::filters::network::downstream_watermark_stats::v3::
             DownstreamWatermarkStats& proto_config,
         Stats::Scope& scope);

  FilterStats& stats() { return stats_; }

private:
  static FilterStats generateStats(absl::string_view prefix, Stats::Scope& scope);

  FilterStats stats_;
};

using ConfigSharedPtr = std::shared_ptr<Config>;

class Filter : public Network::ReadFilter,
               public Network::ConnectionCallbacks,
               Logger::Loggable<Logger::Id::filter> {
public:
  Filter(const ConfigSharedPtr& config) : config_(config) {}
  ~Filter() override;

  // Network::ReadFilter
  Network::FilterStatus onData(Buffer::Instance&, bool) override {
    return Network::FilterStatus::Continue;
  }
  Network::FilterStatus onNewConnection() override { return Network::FilterStatus::Continue; }
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks& read_callbacks) override;

  // Network::ConnectionCallbacks
  void onEvent(Network::ConnectionEvent event) override;
  void onAboveWriteBufferHighWatermark() override;
  void onBelowWriteBufferLowWatermark() override;

private:
  void clearAboveHighWatermark();

  const ConfigSharedPtr config_;
  Network::ReadFilterCallbacks* read_callbacks_{};
  bool above_high_watermark_{false};
};

} // namespace DownstreamWatermarkStats
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
