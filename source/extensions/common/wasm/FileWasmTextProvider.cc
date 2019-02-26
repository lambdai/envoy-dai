#include "extentions/common/wasm/file_wasm_text_provider.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace Wasm {

FileWasmTextProvider::FileWasmTextProvider() {
}

FileWasmTextProvider::~WasmTextProvider() = default;

const WasmText* FileWasmTextProvider::getWasmText() const {
  return nullptr;
}
}
}
}
}
