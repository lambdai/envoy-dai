#pragma once

#include "envoy/stream_info/filter_state.h"

namespace Envoy {
namespace Network {

/**
 * Tunnel info of the stream. It is stored by the downstream and referenced by the upstream when an
 * upstream connection is upon establishment.
 */
class TunnelFilterState : public StreamInfo::FilterState::Object {
public:
  using TransparentTunnelData = std::map<std::string, std::string>;
  TunnelFilterState(TransparentTunnelData tunnel_data) : tunnel_data_(tunnel_data) {}
  const TransparentTunnelData& value() const { return tunnel_data_; }
  static const std::string& key();

private:
  const TransparentTunnelData tunnel_data_;
};

} // namespace Network
} // namespace Envoy
