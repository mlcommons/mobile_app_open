package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

cc_library(
    name = "libsamsungbackend.so",
    srcs = select({
        "android_arm64": [
            "sbe_core.cc",
            "sbe_helper.cc",
        ],
    }),
    hdrs = [
        "backend_c.h",
        "sbe_config.hpp",
        "sbe_core.hpp",
        "sbe_helper.hpp",
        "sbe_loader.hpp",
        "sbe_utils.hpp",
        "type.h",
    ],
)
