#pragma once

#include "extensions/common/wasm/wasm_text_provider.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace Wasm {
class FileWasmTextProvider : public WasmTextProvider {
 public:
  FileWasmTextProvider();
  virtual ~FileWasmTextProvider() override;
  virtual const WasmText* getWasmText() override;
};
}
}
}
}
