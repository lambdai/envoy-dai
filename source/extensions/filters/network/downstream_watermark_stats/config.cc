#include "source/extensions/filters/network/downstream_watermark_stats/config.h"

#include "source/extensions/filters/network/downstream_watermark_stats/downstream_watermark_stats.h"
#include "source/extensions/filters/network/well_known_names.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace DownstreamWatermarkStats {

Network::FilterFactoryCb DownstreamWatermarkStatsConfigFactory::createFilterFactoryFromProtoTyped(
    const envoy::extensions::filters::network::downstream_watermark_stats::v3::
        DownstreamWatermarkStats& proto_config,
    Server::Configuration::FactoryContext& context) {
  auto config = std::make_shared<Config>(proto_config, context.scope());
  return [config](Network::FilterManager& filter_manager) -> void {
    filter_manager.addReadFilter(std::make_shared<Filter>(config));
  };
}

REGISTER_FACTORY(DownstreamWatermarkStatsConfigFactory,
                 Server::Configuration::NamedNetworkFilterConfigFactory);

} // namespace DownstreamWatermarkStats
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
