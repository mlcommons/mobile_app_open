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
            "mbe_core.cc",
            "mbe_helper.cc",
        ],
    }),
    hdrs = [
        "backend_c.h",
        "mbe_config.hpp",
        "mbe_config_1200.hpp",
        "mbe_config_2100.hpp",
        "mbe_config_2200.hpp",
        "mbe_config_2300.hpp",
        "mbe_core.hpp",
        "mbe_helper.hpp",
        "mbe_loader.hpp",
        "mbe_model_container.hpp",
        "mbe_utils.hpp",
        "type.h",
    ],
)
