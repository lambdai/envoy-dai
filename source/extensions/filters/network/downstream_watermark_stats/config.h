#pragma once

#include "envoy/extensions/filters/network/downstream_watermark_stats/v3/downstream_watermark_stats.pb.h"
#include "envoy/extensions/filters/network/downstream_watermark_stats/v3/downstream_watermark_stats.pb.validate.h"

#include "source/extensions/filters/network/common/factory_base.h"
#include "source/extensions/filters/network/well_known_names.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DownstreamWatermarkStats {

class DownstreamWatermarkStatsConfigFactory
    : public Common::FactoryBase<envoy::extensions::filters::network::downstream_watermark_stats::
                                     v3::DownstreamWatermarkStats> {
public:
  DownstreamWatermarkStatsConfigFactory()
      : FactoryBase(NetworkFilterNames::get().DownstreamWatermarkStats) {}

private:
  Network::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoy::extensions::filters::network::downstream_watermark_stats::v3::
          DownstreamWatermarkStats& proto_config,
      Server::Configuration::FactoryContext& context) override;
};

} // namespace DownstreamWatermarkStats
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
