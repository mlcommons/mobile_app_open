# Link to .so libs
# https://github.com/mlcommons/mobile_back_samsung/tree/merge-submission-v2.0-into-master_samsung-backend/mobile_back_samsung/samsung/lib
package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

config_setting(
    name = "android_arm64",
    values = {"cpu": "arm64-v8a"},
)

cc_library(
    name = "samsungbackend",
    srcs = [
        "samsung_backend/lib/libsamsungbackend.so",
    ] + select({
        "android_arm64": [
            "samsung_backend/lib/libenn_public_api_cpp.so",
            "samsung_backend/lib/libsbe1200_core.so",
            "samsung_backend/lib/libsbe2200_core.so",
            "samsung_backend/lib/libsbe2300_core.so",
            "samsung_backend/lib/libc++.so",
            "samsung_backend/lib/libeden_nn_on_system.so",
        ],
        "//conditions:default": [],
    }),
)
