#include <string>

#include "envoy/registry/registry.h"
#include "envoy/server/filter_config.h"

#include "extensions/filters/listener/tls_inspector/tls_inspector.h"
#include "extensions/filters/listener/well_known_names.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace TlsInspector {

/**
 * Config registration for the TLS inspector filter. @see NamedNetworkFilterConfigFactory.
 */
class TlsInspectorConfigFactory : public Server::Configuration::NamedListenerFilterConfigFactory {
public:
  // NamedListenerFilterConfigFactory
  Network::ListenerFilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message& message,
                               Server::Configuration::ListenerFactoryContext& context) override {
    auto proto_config = MessageUtil::downcastAndValidate<
        const envoy::config::filter::listener::tls_inspector::v2alpha1::TlsInspector&>(message);

    auto config = std::make_shared<Config>(context.scope(), );
    return [config = std::move(config)](Network::ListenerFilterManager& filter_manager) -> void {
      filter_manager.addAcceptFilter(std::make_unique<Filter>(config));
    };
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<
        envoy::config::filter::listener::tls_inspector::v2alpha1::TlsInspector>();
  }

  std::string name() override { return ListenerFilterNames::get().TlsInspector; }
};

/**
 * Static registration for the TLS inspector filter. @see RegisterFactory.
 */
REGISTER_FACTORY(TlsInspectorConfigFactory,
                 Server::Configuration::NamedListenerFilterConfigFactory);

} // namespace TlsInspector
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
