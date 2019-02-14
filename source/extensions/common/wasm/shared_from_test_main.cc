#include <memory>
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

int  main() {
	auto pshare = std::make_shared<ShareNoExplicitCtorNeedArg>(1);
	auto other = pshare;
	return 0;
   
}
