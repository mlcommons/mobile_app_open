cc_library(
    name = "main_c",
    srcs = ["main_c.cc"],
    hdrs = [
        "utils.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//flutter/cpp/c:headers",
        "@openvino//:openvino",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "libbackend.dll",
    linkopts = ["-shared"],
    linkshared = 1,
    win_def_file = "//flutter/cpp/c:dll_export.def",
    deps = [
        ":main_c",
    ],
)
