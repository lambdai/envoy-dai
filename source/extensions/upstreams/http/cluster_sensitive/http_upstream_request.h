#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "envoy/http/codes.h"
#include "envoy/http/conn_pool.h"

#include "common/common/logger.h"
#include "common/config/well_known_names.h"
#include "common/http/header_map_impl.h"
#include "common/router/upstream_request.h"

#include "extensions/upstreams/http/http/upstream_request.h"

namespace Envoy {
namespace Extensions {
namespace Upstreams {
namespace Http {
namespace ClusterSensitive {

class HttpConnPool : public Router::GenericConnPool, public Envoy::Http::ConnectionPool::Callbacks {
public:
  // GenericConnPool
  HttpConnPool(Upstream::ClusterManager& cm, bool is_connect, const Router::RouteEntry& route_entry,
               absl::optional<Envoy::Http::Protocol> downstream_protocol,
               Upstream::LoadBalancerContext* ctx) {
    ASSERT(!is_connect);
    conn_pool_ = cm.httpConnPoolForCluster(route_entry.clusterName(), route_entry.priority(),
                                           downstream_protocol, ctx);
  }
  void newStream(Router::GenericConnectionPoolCallbacks* callbacks) override;
  bool cancelAnyPendingStream() override;
  absl::optional<Envoy::Http::Protocol> protocol() const override;

  // Http::ConnectionPool::Callbacks
  void onPoolFailure(ConnectionPool::PoolFailureReason reason,
                     absl::string_view transport_failure_reason,
                     Upstream::HostDescriptionConstSharedPtr host) override;
  void onPoolReady(Envoy::Http::RequestEncoder& callbacks_encoder,
                   Upstream::HostDescriptionConstSharedPtr host,
                   const StreamInfo::StreamInfo& info) override;
  Upstream::HostDescriptionConstSharedPtr host() const override { return conn_pool_->host(); }

  bool valid() { return conn_pool_ != nullptr; }

private:
  // Points to the actual connection pool to create streams from.
  Envoy::Http::ConnectionPool::Instance* conn_pool_{};
  Envoy::Http::ConnectionPool::Cancellable* conn_pool_stream_handle_{};
  Router::GenericConnectionPoolCallbacks* callbacks_{};
};

class HttpUpstream : public Router::GenericUpstream {
public:
  HttpUpstream(Router::UpstreamToDownstream& upstream_request, Envoy::Http::RequestEncoder* encoder,
               Upstream::HostDescriptionConstSharedPtr host)
      : sub_upstream_(upstream_request, encoder), host_(host) {}

  // GenericUpstream
  void encodeData(Buffer::Instance& data, bool end_stream) override {
    sub_upstream_.encodeData(data, end_stream);
  }

  void encodeMetadata(const Envoy::Http::MetadataMapVector& metadata_map_vector) override {
    sub_upstream_.encodeMetadata(metadata_map_vector);
  }
  
  void encodeHeaders(const Envoy::Http::RequestHeaderMap& headers, bool end_stream) override {
    auto dup = Envoy::Http::RequestHeaderMapImpl::create();
    Envoy::Http::HeaderMapImpl::copyFrom(*dup, headers);
    dup->setCopy(Envoy::Http::LowerCaseString("X-istio-test"), "lambdai");
    // Sanitize original port header.
    dup->remove(Envoy::Http::LowerCaseString("X-istio-original-port"));
    if (auto filter_metadata = host_->cluster().metadata().filter_metadata().find("istio");
        filter_metadata != host_->cluster().metadata().filter_metadata().end()) {
      ENVOY_LOG_MISC(warn, "lambdai: find filter_metadata from {}",
                     host_->cluster().metadata().DebugString());
      const ProtobufWkt::Struct& data_struct = filter_metadata->second;
      const auto& fields = data_struct.fields();
      if (auto iter = fields.find("original_port"); iter != fields.end()) {
        if (iter->second.kind_case() == ProtobufWkt::Value::kStringValue) {
          dup->setCopy(Envoy::Http::LowerCaseString("X-istio-original-port"),
                       iter->second.string_value());
        }
      }
    }
    sub_upstream_.encodeHeaders(*dup, end_stream);
  }

  void encodeTrailers(const Envoy::Http::RequestTrailerMap& trailers) override {
    sub_upstream_.encodeTrailers(trailers);
  }

  void readDisable(bool disable) override { sub_upstream_.readDisable(disable); }

  void resetStream() override { sub_upstream_.resetStream(); }

private:
  Upstreams::Http::Http::HttpUpstream sub_upstream_;
  Upstream::HostDescriptionConstSharedPtr host_{};
};

} // namespace ClusterSensitive
} // namespace Http
} // namespace Upstreams
} // namespace Extensions
} // namespace Envoy
