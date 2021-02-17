#include "common/common/macros.h"
#include "exe/main_common.h"
#include <iostream>
// NOLINT(namespace-envoy)

/**
 * Basic Site-Specific main()
 *
 * This should be used to do setup tasks specific to a particular site's
 * deployment such as initializing signal handling. It calls main_common
 * after setting up command line options.
 */
int main(int argc, char** argv) {
  auto pnew = new (int)(7);
  pnew = nullptr; // The memory is leaked here.
  auto pmalloc = malloc(7);
  pmalloc = nullptr; // The memory is leaked here.
  std::cout << "foo: after malloc and new" << std::endl;
  return Envoy::MainCommon::main(argc, argv);
  // UNREFERENCED_PARAMETER(argc);
  // UNREFERENCED_PARAMETER(argv);
  // return 0;
}
