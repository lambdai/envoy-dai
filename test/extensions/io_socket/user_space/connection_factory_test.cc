#include "common/network/address_impl.h"

#include "extensions/io_socket/user_space/connection_factory.h"

#include "test/test_common/registry.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::NiceMock;

namespace Envoy {
namespace Extensions {
namespace IoSocket {
namespace UserSpace {
namespace {

class FactoryTest : public testing::Test {
public:
  FactoryTest()
      : api_(Api::createApiForTest()), dispatcher_(api_->allocateDispatcher("test_thread")),
        registration_(factory_) {}

  Api::ApiPtr api_;
  Event::DispatcherPtr dispatcher_;
  UserSpace::ClientConnectionFactoryImpl factory_;
  Registry::InjectFactory<Network::ClientConnectionFactory> registration_;
};

TEST_F(FactoryTest, TestCreate) {
  // TODO(lambdai): share_ptr<const Type> covariance
  auto address = std::dynamic_pointer_cast<const Envoy::Network::Address::Instance>(
      std::make_shared<const Network::Address::EnvoyInternalInstance>("abc"));
  auto* factory = Network::ClientConnectionFactory::getFactoryByAddress(address);
  // Registered.
  ASSERT_NE(nullptr, factory);
  auto connection =
      factory->createClientConnection(*dispatcher_, address, address, nullptr, nullptr);
  // Naive implementation.
  ASSERT_EQ(nullptr, connection);
}
} // namespace
} // namespace UserSpace
} // namespace IoSocket
} // namespace Extensions
} // namespace Envoy