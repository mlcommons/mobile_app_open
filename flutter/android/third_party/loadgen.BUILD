package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])

exports_files(["LICENSE"])

cc_library(
    name = "loadgen",
    srcs = [
        "loadgen/early_stopping.cc",
        "loadgen/issue_query_controller.cc",
        "loadgen/loadgen.cc",
        "loadgen/logging.cc",
        "loadgen/logging.h",
        "loadgen/test_settings_internal.cc",
        "loadgen/test_settings_internal.h",
        "loadgen/utils.cc",
        "loadgen/utils.h",
        "loadgen/version.cc",
        "loadgen/version.h",
        "loadgen/version_generated.cc",
    ],
    hdrs = [
        "loadgen/early_stopping.h",
        "loadgen/issue_query_controller.h",
        "loadgen/loadgen.h",
        "loadgen/query_sample.h",
        "loadgen/query_sample_library.h",
        "loadgen/results.h",
        "loadgen/system_under_test.h",
        "loadgen/test_settings.h",
    ],
)
