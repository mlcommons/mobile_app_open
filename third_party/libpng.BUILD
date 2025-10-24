# Description:
#   libpng is the official PNG reference library.

licenses(["notice"])  # BSD/MIT-like license

genrule(
    name = "pnglibconf",
    srcs = ["scripts/pnglibconf.h.prebuilt"],
    outs = ["pnglibconf.h"],
    cmd = "cp $(location scripts/pnglibconf.h.prebuilt) $(location pnglibconf.h)",
)

config_setting(
    name = "target_arm64",
    constraint_values = ["@platforms//cpu:arm64"],
)

# Apple iOS CPU-based config_settings to avoid host/target mix-ups under split transitions.
config_setting(
    name = "cpu_ios_arm64",
    values = {"cpu": "ios_arm64"},
)

cc_library(
    name = "png",
    srcs = [
        "png.c",
        "pngdebug.h",
        "pngerror.c",
        "pngget.c",
        "pnginfo.h",
        ":pnglibconf",
        "pngmem.c",
        "pngpread.c",
        "pngpriv.h",
        "pngread.c",
        "pngrio.c",
        "pngrtran.c",
        "pngrutil.c",
        "pngset.c",
        "pngstruct.h",
        "pngtrans.c",
        "pngwio.c",
        "pngwrite.c",
        "pngwtran.c",
        "pngwutil.c",
    ] + select({
        ":cpu_ios_arm64": [
            "arm/arm_init.c",
            "arm/filter_neon_intrinsics.c",
            "arm/palette_neon_intrinsics.c",
        ],
        "//conditions:default": [],
    }),
    hdrs = [
        "png.h",
        "pngconf.h",
    ],
    includes = ["."],
    linkopts = select({
        "@bazel_tools//src/conditions:windows": [],
        "//conditions:default": ["-lm"],
    }),
    copts = select({
        ":cpu_ios_arm64": ["-DPNG_ARM_NEON_OPT=2"],
        "//conditions:default": ["-DPNG_ARM_NEON_OPT=0"],
    }),
    visibility = ["//visibility:public"],
    deps = ["@zlib//:zlib"],
)
