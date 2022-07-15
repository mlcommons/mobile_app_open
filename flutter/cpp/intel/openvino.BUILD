cc_library(
    name = "openvino",
    srcs = glob(["**/*.lib"]) + glob(["**/*.dll"]),
    hdrs = glob(["**/*.hpp"]),
    includes = [
        "src/core/include/",
        "src/inference/include/",
        "src/inference/include/ie/",
    ],
    visibility = ["//visibility:public"],
)
