load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_extension",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

# tap wrapper around a transport socket.

envoy_extension_package()

envoy_cc_extension(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = [
        "//envoy/server:transport_socket_config_interface",
        "//source/common/config:utility_lib",
        "@envoy_api//envoy/extensions/transport_sockets/internal_tunnel/v3:pkg_cc_proto",
    ],
)
