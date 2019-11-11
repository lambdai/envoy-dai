#include "server/tag_generator_batch_impl.h"

namespace Envoy {
namespace Server {
TagGeneratorBatchImpl::~TagGeneratorBatchImpl() = default;

std::vector<int64_t> TagGeneratorBatchImpl::addFilterChains(
    absl::Span<const ::envoy::api::v2::listener::FilterChain* const> filter_chain_span) {
  std::vector<int64_t> res;
  res.resize(filter_chain_span.size());
  for (const auto& fc : filter_chain_span) {
    const auto& kv = filter_chains_.emplace(*fc, ++next_tag_);
    res.emplace_back(kv.second ? next_tag_ : kv.first->second);
  }
  return res;
}

TagGenerator::Tags TagGeneratorBatchImpl::getTags() {
  TagGenerator::Tags res;
  for (const auto& kv : filter_chains_) {
    res.insert(kv.second);
  }
  return res;
}

} // namespace Server
} // namespace Envoy
