#include <memory>
#include "gtest/gtest.h"
class ShareDefault : public std::enable_shared_from_this<ShareDefault> {
};

class ShareNoExplicitEnableSharedFromThis : public std::enable_shared_from_this<ShareNoExplicitEnableSharedFromThis> {
 public:
  ShareNoExplicitEnableSharedFromThis() = default;
  ShareNoExplicitEnableSharedFromThis(const ShareNoExplicitEnableSharedFromThis&) /*: enable_shared_from_this<>() */ {}
};

class ShareNoExplicitCtorNeedArg : public std::enable_shared_from_this<ShareNoExplicitCtorNeedArg> {
 public:
  ShareNoExplicitCtorNeedArg(int i) : i_(i) {}
  ShareNoExplicitCtorNeedArg(const ShareNoExplicitCtorNeedArg&) /*: enable_shared_from_this<>() */ {}
 private:
  int i_;
};

class ShareFoo : public std::enable_shared_from_this<ShareFoo> {
 public:
  ShareFoo() = default;
  ShareFoo(const ShareFoo&) : enable_shared_from_this<ShareFoo>() {}
};

class ShareBar : public std::enable_shared_from_this<ShareBar> {
 public:
  ShareBar() = default;
  ShareBar(const ShareBar& that) : enable_shared_from_this<ShareBar>(that) {}
};

TEST(ShareDefaultTest, copyShareNoExplicitEnableSharedFromThis) {
  using ShareStruct = ShareNoExplicitEnableSharedFromThis;
  auto pShare = std::make_shared<ShareStruct>();
  EXPECT_EQ(1, pShare.use_count());
  auto copyShare = std::make_shared<ShareStruct>(*pShare);
  ASSERT_EQ(1, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
  auto copyPtr = pShare;
  ASSERT_EQ(2, copyPtr.use_count());
  ASSERT_EQ(2, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
}

TEST(ShareDefaultTest, copyShareNoExplicitCtorNeedArg) {
  using ShareStruct = ShareNoExplicitCtorNeedArg;
  auto pShare = std::make_shared<ShareStruct>(1);
  EXPECT_EQ(1, pShare.use_count());
  auto copyShare = std::make_shared<ShareStruct>(*pShare);
  ASSERT_EQ(1, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
  auto copyPtr = pShare;
  ASSERT_EQ(2, copyPtr.use_count());
  ASSERT_EQ(2, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
}

TEST(ShareDefaultTest, copyShareDefault) {
  using ShareStruct = ShareDefault;
  auto pShare = std::make_shared<ShareStruct>();
  EXPECT_EQ(1, pShare.use_count());
  auto copyShare = std::make_shared<ShareStruct>(*pShare);
  ASSERT_EQ(1, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
  auto copyPtr = pShare;
  ASSERT_EQ(2, copyPtr.use_count());
  ASSERT_EQ(2, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
}

TEST(ShareFooTest, copyShareFoo) {
  using ShareStruct = ShareFoo;
  auto pShare = std::make_shared<ShareStruct>();
  EXPECT_EQ(1, pShare.use_count());
  auto copyShare = std::make_shared<ShareStruct>(*pShare);
  ASSERT_EQ(1, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
  auto copyPtr = pShare;
  ASSERT_EQ(2, copyPtr.use_count());
  ASSERT_EQ(2, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
}

TEST(ShareBarTest, copyShareBar) {
  using ShareStruct = ShareBar;
  auto pShare = std::make_shared<ShareStruct>();
  EXPECT_EQ(1, pShare.use_count());
  auto copyShare = std::make_shared<ShareStruct>(*pShare);
  EXPECT_EQ(1, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
  auto copyPtr = pShare;
  ASSERT_EQ(2, copyPtr.use_count());
  ASSERT_EQ(2, pShare.use_count());
  ASSERT_EQ(1, copyShare.use_count());
}
