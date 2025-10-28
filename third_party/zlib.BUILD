# Minimal BUILD for zlib used by TensorFlow and this project.

cc_library(
    name = "zlib",
    srcs = [
        "adler32.c",
        "compress.c",
        "crc32.c",
        "deflate.c",
        "gzclose.c",
        "gzlib.c",
        "gzread.c",
        "gzwrite.c",
        "infback.c",
        "inflate.c",
        "inffast.c",
        "inftrees.c",
        "trees.c",
        "uncompr.c",
        "zutil.c",
    ],
    hdrs = [
        "zconf.h",
        "zlib.h",
    ],
    includes = ["."],
    # On POSIX (darwin, linux, android) unistd.h exists; on Windows it doesn't.
    copts = select({
        "@bazel_tools//src/conditions:windows": [],
        "//conditions:default": ["-DZ_HAVE_UNISTD_H"],
    }),
    visibility = ["//visibility:public"],
)
