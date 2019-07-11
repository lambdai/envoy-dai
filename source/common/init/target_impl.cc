#include "common/init/target_impl.h"

namespace Envoy {
namespace Init {

TargetHandleImpl::TargetHandleImpl(absl::string_view handle_name, absl::string_view name,
                                   std::weak_ptr<InternalInitalizeFn> fn)
    : handle_name_(handle_name), name_(name), fn_(std::move(fn)) {}

bool TargetHandleImpl::initialize(const Watcher& watcher) const {
  auto locked_fn(fn_.lock());
  if (locked_fn) {
    // If we can "lock" a shared pointer to the target's callback function, call it
    // with a new handle to the ManagerImpl's watcher that was passed in.
    ENVOY_LOG(debug, "{} initializing {}", handle_name_, name_);
    (*locked_fn)(watcher.createHandle(name_));
    return true;
  } else {
    // If not, the target was already destroyed.
    ENVOY_LOG(debug, "{} can't initialize {} (unavailable)", handle_name_, name_);
    return false;
  }
}

TargetImplBase::TargetImplBase(absl::string_view name, std::shared_ptr<InternalInitalizeFn> fn)
    : name_(fmt::format("target {}", name)), fn_(fn) {}
absl::string_view TargetImplBase::name() const { return name_; }

bool TargetImplBase::ready() {
  doReady();
  if (watcher_handle_) {
    // If we have a handle for the ManagerImpl's watcher, signal it and then reset so it can't be
    // accidentally signaled again.
    const bool result = watcher_handle_->ready();
    watcher_handle_.reset();
    return result;
  }
  return false;
}

TargetHandlePtr TargetImplBase::createHandle(absl::string_view handle_name) const {
  // Note: can't use std::make_unique here because TargetHandleImpl ctor is private.
  return std::unique_ptr<TargetHandle>(
      new TargetHandleImpl(handle_name, name_, std::weak_ptr<InternalInitalizeFn>(fn_)));
}
TargetImplBase::~TargetImplBase() { ENVOY_LOG(debug, "{} destroyed", name_); }

TargetImpl::TargetImpl(absl::string_view name, InitializeFn fn)
    : TargetImplBase(
          name, std::make_shared<InternalInitalizeFn>([this, fn](WatcherHandlePtr watcher_handle) {
            watcher_handle_ = std::move(watcher_handle);
            fn();
          })) {}

void TargetImpl::doReady() {}

EagerTargetImpl::EagerTargetImpl(absl::string_view name)
    : TargetImplBase(name,
                     std::make_shared<InternalInitalizeFn>([this](WatcherHandlePtr watcher_handle) {
                       if (is_ready_) {
                         watcher_handle->ready();
                       } else {
                         watcher_handle_ = std::move(watcher_handle);
                       }
                     })) {}

void EagerTargetImpl::doReady() { is_ready_ = true; }

} // namespace Init
} // namespace Envoy
