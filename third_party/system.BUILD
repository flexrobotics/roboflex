load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])


#-----------------------------------------------#
# Third party libraries installed on the system #
#-----------------------------------------------#

cc_library(
    name = "zmq",
    srcs = ["usr/lib/x86_64-linux-gnu/libzmq.so"],
    hdrs = ["usr/include/zmq.h"],
    includes = ["usr/include"],
)

cc_library(
    name = "libsdl2",
    srcs = ["lib/x86_64-linux-gnu/libSDL2-2.0.so"],
    hdrs = glob(["usr/include/SDL2/**"]),
    includes = ["usr/include"],
)