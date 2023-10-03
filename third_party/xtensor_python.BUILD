load("@pybind11_bazel//:build_defs.bzl", "pybind_library")

pybind_library(
    name = "xtensor_python",
    hdrs = glob(["include/xtensor-python/*.hpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@xtensor",
        "@numpy_headers_local//:numpy_headers",
    ],
)