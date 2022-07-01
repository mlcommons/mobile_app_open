package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

load("@rules_cc//cc:defs.bzl", "objc_library")

objc_library(
    name = "applelib",
    srcs = [
        "cpp/backend_apple/main.mm",
    ],
    hdrs = glob(["cpp/backend_apple/*.h"]),
    deps = [
        "//flutter/cpp/c:headers",
    ],
    alwayslink = 1,
)
