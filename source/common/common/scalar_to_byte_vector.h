#pragma once

#include <cinttypes>
#include <vector>

namespace Envoy {

template <typename T> void pushScalarToByteVector(T val, std::vector<uint8_t>& bytes) {
  uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(&val);
  for (uint32_t byte_index = 0; byte_index < sizeof val; byte_index++) {
    bytes.push_back(*byte_ptr++);
  }
}

template <typename T>
void pushScalarSpanToVector(const std::vector<T>& from, std::vector<uint8_t>& bytes) {
  for (const auto& t : from) {
    pushScalarToByteVector(t, bytes);
  }
}

} // namespace Envoy
