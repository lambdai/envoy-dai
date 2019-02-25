#pragma once

#include "envoy/common/pure.h"
#include "extensions/common/wasm/wasm_text.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace Wasm {
class WasmTextProvider {
 public:
  virtual ~WasmTextProvider() = default;
  virtual const WasmText* getWasmText() const PURE;
};
}
}
}
}
