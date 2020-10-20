#include "common/buffer/buffer_impl.h"
#include "common/network/address_impl.h"
#include "common/router/config_impl.h"
#include "common/router/router.h"
#include "common/router/upstream_request.h"

#include "extensions/upstreams/http/cluster_sensitive/http_upstream_request.h"

#include "test/common/http/common.h"
#include "test/mocks/common.h"
#include "test/mocks/router/mocks.h"
#include "test/mocks/router/router_filter_interface.h"
#include "test/mocks/server/factory_context.h"
#include "test/mocks/server/instance.h"
#include "test/mocks/tcp/mocks.h"
#include "test/test_common/utility.h"
#include "test/mocks/upstream/host.h"
#include "test/mocks/http/stream_encoder.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using Envoy::Http::TestRequestHeaderMapImpl;
using Envoy::Router::UpstreamRequest;
using testing::_;
using testing::AnyNumber;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace Upstreams {
namespace Http {
namespace ClusterSensitive {

// class ClusterSensitiveConnPoolTest : public ::testing::Test {
// public:
//   ClusterSensitiveConnPoolTest() : host_(std::make_shared<NiceMock<Upstream::MockHost>>()) {
//     NiceMock<Router::MockRouteEntry> route_entry;
//     NiceMock<Upstream::MockClusterManager> cm;
//     EXPECT_CALL(cm, tcpConnPoolForCluster(_, _, _)).WillOnce(Return(&mock_pool_));
//     conn_pool_ = std::make_unique<ClusterSensitive>(cm, true, route_entry,
//                                                     Envoy::Http::Protocol::Http11, nullptr);
//   }

//   std::unique_ptr<TcpConnPool> conn_pool_;
//   Envoy::Tcp::ConnectionPool::MockInstance mock_pool_;
//   Router::MockGenericConnectionPoolCallbacks mock_generic_callbacks_;
//   std::shared_ptr<NiceMock<Upstream::MockHost>> host_;
//   NiceMock<Envoy::ConnectionPool::MockCancellable> cancellable_;
// };

// TEST_F(ClusterSensitiveConnPoolTest, Basic) {
//   NiceMock<Network::MockClientConnection> connection;

//   EXPECT_CALL(mock_pool_, newConnection(_)).WillOnce(Return(&cancellable_));
//   conn_pool_->newStream(&mock_generic_callbacks_);

//   EXPECT_CALL(mock_generic_callbacks_, upstreamToDownstream());
//   EXPECT_CALL(mock_generic_callbacks_, onPoolReady(_, _, _, _));
//   auto data = std::make_unique<NiceMock<Envoy::Tcp::ConnectionPool::MockConnectionData>>();
//   EXPECT_CALL(*data, connection()).Times(AnyNumber()).WillRepeatedly(ReturnRef(connection));
//   conn_pool_->onPoolReady(std::move(data), host_);
// }

// TEST_F(ClusterSensitiveConnPoolTest, OnPoolFailure) {
//   EXPECT_CALL(mock_pool_, newConnection(_)).WillOnce(Return(&cancellable_));
//   conn_pool_->newStream(&mock_generic_callbacks_);

//   EXPECT_CALL(mock_generic_callbacks_, onPoolFailure(_, _, _));
//   conn_pool_->onPoolFailure(Envoy::Tcp::ConnectionPool::PoolFailureReason::LocalConnectionFailure,
//                             host_);

//   // Make sure that the pool failure nulled out the pending request.
//   EXPECT_FALSE(conn_pool_->cancelAnyPendingStream());
// }

// TEST_F(ClusterSensitiveConnPoolTest, Cancel) {
//   // Initially cancel should fail as there is no pending request.
//   EXPECT_FALSE(conn_pool_->cancelAnyPendingStream());

//   EXPECT_CALL(mock_pool_, newConnection(_)).WillOnce(Return(&cancellable_));
//   conn_pool_->newStream(&mock_generic_callbacks_);

//   // Canceling should now return true as there was an active request.
//   EXPECT_TRUE(conn_pool_->cancelAnyPendingStream());

//   // A second cancel should return false as there is not a pending request.
//   EXPECT_FALSE(conn_pool_->cancelAnyPendingStream());
// }

class HttpUpstreamTest : public ::testing::Test {
public:
  HttpUpstreamTest() {
    cluster_metadata_ = std::make_shared<envoy::config::core::v3::Metadata>(
        TestUtility::parseYaml<envoy::config::core::v3::Metadata>(
            R"EOF(
        filter_metadata:
          istio:
            original_port: "80"
      )EOF"));
  }

  ~HttpUpstreamTest() override {}

protected:
  Router::MockUpstreamToDownstream mock_upstream_to_downstream_;
  // NiceMock<Router::MockRouterFilterInterface> mock_router_filter_;
  // std::unique_ptr<HttpUpstream> http_upstream_;
  TestRequestHeaderMapImpl request_{{":method", "CONNECT"},
                                    {":path", "/"},
                                    {":protocol", "bytestream"},
                                    {":scheme", "https"},
                                    {":authority", "host"}};
  std::shared_ptr<envoy::config::core::v3::Metadata> cluster_metadata_;
};

TEST_F(HttpUpstreamTest, TestAddedHeader) {
  NiceMock<Envoy::Http::MockRequestEncoder> encoder;
  std::shared_ptr<NiceMock<Upstream::MockHost>> host =
      std::make_shared<NiceMock<Upstream::MockHost>>();
  std::shared_ptr<Upstream::MockClusterInfo> info{
      new ::testing::NiceMock<Upstream::MockClusterInfo>()};
  ON_CALL(*host, cluster()).WillByDefault(ReturnRef(*info));
  Envoy::Http::TestRequestHeaderMapImpl headers{};
  HttpTestUtility::addDefaultHeaders(headers);
  auto http_upstream = std::make_unique<HttpUpstream>(mock_upstream_to_downstream_, &encoder, host);
  EXPECT_CALL(encoder, encodeHeaders(HeaderHasValueRef("x-istio-test", "lambdai"), false));
  http_upstream->encodeHeaders(headers, false);
}

TEST_F(HttpUpstreamTest, TestAddClusterInfo) {
  NiceMock<Envoy::Http::MockRequestEncoder> encoder;
  std::shared_ptr<NiceMock<Upstream::MockHost>> host =
      std::make_shared<NiceMock<Upstream::MockHost>>();
  std::shared_ptr<Upstream::MockClusterInfo> info{
      new ::testing::NiceMock<Upstream::MockClusterInfo>()};
  ON_CALL(*host, cluster()).WillByDefault(ReturnRef(*info));
  ON_CALL(*info, metadata()).WillByDefault(ReturnRef(*cluster_metadata_));
  auto http_upstream = std::make_unique<HttpUpstream>(mock_upstream_to_downstream_, &encoder, host);
  Envoy::Http::TestRequestHeaderMapImpl headers{};
  HttpTestUtility::addDefaultHeaders(headers);
  EXPECT_CALL(encoder, encodeHeaders(HeaderHasValueRef("X-istio-original-port", "80"), false));
  http_upstream->encodeHeaders(headers, false);
}

TEST_F(HttpUpstreamTest, TestBasicFlow) {
  NiceMock<Envoy::Http::MockRequestEncoder> encoder;
  std::shared_ptr<NiceMock<Upstream::MockHost>> host =
      std::make_shared<NiceMock<Upstream::MockHost>>();
  std::shared_ptr<Upstream::MockClusterInfo> info{
      new ::testing::NiceMock<Upstream::MockClusterInfo>()};
  ON_CALL(*host, cluster()).WillByDefault(ReturnRef(*info));
  Envoy::Http::TestRequestHeaderMapImpl headers{};
  HttpTestUtility::addDefaultHeaders(headers);
  auto http_upstream = std::make_unique<HttpUpstream>(mock_upstream_to_downstream_, &encoder, host);
  {
    EXPECT_CALL(encoder, encodeHeaders(HeaderHasValueRef("x-istio-test", "lambdai"), false));
    http_upstream->encodeHeaders(headers, false);

    auto metadata_map_ptr = std::make_unique<Envoy::Http::MetadataMap>();
    metadata_map_ptr->emplace("key", "value");
    Envoy::Http::MetadataMapVector metadata_map_vector;
    metadata_map_vector.push_back(std::move(metadata_map_ptr));
    EXPECT_CALL(encoder, encodeMetadata(testing::Ref(metadata_map_vector)));
    http_upstream->encodeMetadata(metadata_map_vector);

    Buffer::OwnedImpl body("abc");
    EXPECT_CALL(encoder, encodeData(BufferEqual(&body), false));
    http_upstream->encodeData(body, false);

    Envoy::Http::TestRequestTrailerMapImpl trailers{{"some", "request_trailer"}};
    EXPECT_CALL(encoder, encodeTrailers(HeaderMapEqualRef(&trailers)));
    http_upstream->encodeTrailers(trailers);
  }
}

TEST_F(HttpUpstreamTest, TestReadDisable) {

  NiceMock<Envoy::Http::MockRequestEncoder> encoder;
  std::shared_ptr<NiceMock<Upstream::MockHost>> host =
      std::make_shared<NiceMock<Upstream::MockHost>>();
  std::shared_ptr<Upstream::MockClusterInfo> info{
      new ::testing::NiceMock<Upstream::MockClusterInfo>()};
  ON_CALL(*host, cluster()).WillByDefault(ReturnRef(*info));
  Envoy::Http::TestRequestHeaderMapImpl headers{};
  HttpTestUtility::addDefaultHeaders(headers);
  auto http_upstream = std::make_unique<HttpUpstream>(mock_upstream_to_downstream_, &encoder, host);

  Envoy::Http::MockStream stream;
  ON_CALL(encoder, getStream()).WillByDefault(ReturnRef(stream));
  EXPECT_CALL(stream, readDisable(true));
  http_upstream->readDisable(true);

  EXPECT_CALL(stream, readDisable(false));
  http_upstream->readDisable(false);
}

TEST_F(HttpUpstreamTest, TestResetStream) {
  NiceMock<Envoy::Http::MockRequestEncoder> encoder;
  std::shared_ptr<NiceMock<Upstream::MockHost>> host =
      std::make_shared<NiceMock<Upstream::MockHost>>();
  std::shared_ptr<Upstream::MockClusterInfo> info{
      new ::testing::NiceMock<Upstream::MockClusterInfo>()};
  ON_CALL(*host, cluster()).WillByDefault(ReturnRef(*info));
  Envoy::Http::TestRequestHeaderMapImpl headers{};
  HttpTestUtility::addDefaultHeaders(headers);
  auto http_upstream = std::make_unique<HttpUpstream>(mock_upstream_to_downstream_, &encoder, host);

  Envoy::Http::MockStream stream;
  ON_CALL(encoder, getStream()).WillByDefault(ReturnRef(stream));

  EXPECT_CALL(stream, removeCallbacks(_));
  EXPECT_CALL(stream, resetStream(Envoy::Http::StreamResetReason::LocalReset));
  http_upstream->resetStream();
}
} // namespace ClusterSensitive
} // namespace Http
} // namespace Upstreams
} // namespace Extensions
} // namespace Envoy
