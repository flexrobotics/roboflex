load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "dynamixel_sdk",
    srcs = glob(["c++/src/dynamixel_sdk/*.cpp"]),
    hdrs = glob(["c++/include/dynamixel_sdk/*.h"]),
    includes = ["c++/include/dynamixel_sdk"],
    visibility = ["//visibility:public"],
)
