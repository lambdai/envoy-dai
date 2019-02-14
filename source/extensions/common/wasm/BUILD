licenses(["notice"])  # Apache 2

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_library",
    "envoy_package",
)

load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_fuzz_test",
    "envoy_cc_test",
    "envoy_cc_test_library",
    "envoy_package",
    "envoy_select_hot_restart",
)
load(
    "//source/extensions:all_extensions.bzl",
    "envoy_all_extensions",
)
envoy_package()

envoy_cc_library(
    name = "well_known_names",
    hdrs = ["well_known_names.h"],
    deps = [
        "//source/common/singleton:const_singleton",
    ],
)

envoy_cc_library(
    name = "wasm_hdr",
    hdrs = ["wasm.h"],
    deps = [
        ":well_known_names",
        "//include/envoy/http:codes_interface",
        "//include/envoy/http:filter_interface",
        "//include/envoy/upstream:cluster_manager_interface",
        "//source/extensions/filters/http:well_known_names",
        "@envoy_api//envoy/config/filter/http/wasm/v2:wasm_cc",
    ],
)

envoy_cc_library(
    name = "wasm_lib",
    srcs = ["wasm.cc"],
    deps = [
        ":wasm_hdr",
        "//external:abseil_node_hash_map",
        "//source/common/buffer:buffer_lib",
        "//source/common/common:enum_to_int",
        "//source/common/config:datasource_lib",
        "//source/common/http:message_lib",
        "//source/common/http:utility_lib",
        "//source/extensions/common/wasm/wavm:wavm_lib",
    ],
)

envoy_cc_test(
    name = "shared_from_test",
    srcs = [
        "shared_from_test.cc",
    ],
    deps = [
        #"//testing/base/public:gunit_main",
        "//test/test_common:test_time_lib",
        "//test/test_common:utility_lib",
    ],
)

