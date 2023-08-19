load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "jpeg-compressor",
    hdrs = glob(["*.h"]),
    srcs = [
        "jpgd.cpp",
        "jpge.cpp",
    ],
    includes = ["."],
    include_prefix = "jpeg-compressor",
    visibility = ["//visibility:public"],
)
