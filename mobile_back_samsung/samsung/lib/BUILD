load("//flutter/cpp/proto:pbtxt2header.bzl", "pbtxt2header")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

pbtxt2header(
    name = "mbe_config_pbtxt",
    srcs = [
        "public/include/mbe_config_1200.pbtxt",
        "public/include/mbe_config_2100.pbtxt",
        "public/include/mbe_config_2200.pbtxt",
        "public/include/mbe_config_2300.pbtxt",
        "public/include/mbe_config_2400.pbtxt",
        "public/include/mbe_config_2500.pbtxt",
    ],
)

cc_library(
    name = "samsungbackend",
    srcs = select({
        "android_arm64": [
            "public/mbe_core/mbe_core.cc",
            "public/mbe_core/mbe_helper.cc",
        ],
    }),
    hdrs = [
        "public/include/mbe_config.hpp",
        "public/include/mbe_config_1200.hpp",
        "public/include/mbe_config_2100.hpp",
        "public/include/mbe_config_2200.hpp",
        "public/include/mbe_config_2300.hpp",
        "public/include/mbe_config_2400.hpp",
        "public/include/mbe_config_2500.hpp",
        "public/include/mbe_core_holder.hpp",
        "public/include/mbe_helper.hpp",
        "public/include/mbe_loader.hpp",
        "public/include/mbe_utils.hpp",
    ],
    includes = [
        "include",
        "public/include",
    ],
    linkopts = [
        "-llog",
        "-lc++_shared",
    ],
    deps = [
        ":mbe_config_pbtxt",
        "//flutter/cpp/c:headers",
    ],
)
