load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "implot",
    visibility = ["//visibility:public"],
    includes = ["."],
    hdrs = [
        "implot.h",
        "implot_internal.h",
    ],
    srcs = [
        "implot.cpp",
        "implot_items.cpp",
    ],
    defines = ["IMGUI_DEFINE_MATH_OPERATORS"],
    deps = [
        "@roboflex//third_party:imgui",
    ]
)
