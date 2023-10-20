load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

#-----------------------------------------------#
# Third party libraries installed on the system #
#-----------------------------------------------#

cc_library(
    name = "usb-1.0",
    srcs = ["usr/lib/x86_64-linux-gnu/libusb-1.0.so"],
    hdrs = ["usr/include/libusb-1.0/libusb.h"],
    includes = ["usr/include"],
)