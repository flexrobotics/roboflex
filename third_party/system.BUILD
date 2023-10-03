load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])


#-----------------------------------------------#
# Third party libraries installed on the system #
#-----------------------------------------------#

cc_library(
    name = "libsdl2",
    srcs = ["lib/x86_64-linux-gnu/libSDL2-2.0.so"],
    hdrs = glob(["usr/include/SDL2/**"]),
    includes = ["usr/include", "usr/include/SDL2"],
)

cc_library(
    name = "usb-1.0",
    srcs = ["usr/lib/x86_64-linux-gnu/libusb-1.0.so"],
    hdrs = ["usr/include/libusb-1.0/libusb.h"],
    includes = ["usr/include"],
)

cc_library(
    name = "libjpeg",
    srcs = ["usr/lib/x86_64-linux-gnu/libjpeg.so"],
    hdrs = ["usr/include/jpeglib.h"],
    includes = ["usr/include"],
)

cc_library(
    name = "mosquitto",
    srcs = ["usr/lib/x86_64-linux-gnu/libmosquitto.so"],
    hdrs = ["usr/include/mosquitto.h"],
    includes = ["usr/include"],
)

cc_library(
    name = "alsa",
    srcs = ["usr/lib/x86_64-linux-gnu/libasound.so"],
    hdrs = ["usr/include/alsa/asoundlib.h"],
    includes = ["usr/include"],
)

# new_local_repository(
#     name = "numpy_headers_local",
#     path = "/opt/homebrew/lib/python3.11/site-packages/numpy/core/include",
#     build_file_content = """
# package(default_visibility = ["//visibility:public"])
# cc_library(
#     name = "numpy_headers",
#     srcs = glob(["numpy/*.h"]),
# )
# """
# )

cc_library(
    name = "numpy_headers",
    hdrs = glob(["opt/homebrew/lib/python3.11/site-packages/numpy/core/include"]),
    includes = ["opt/homebrew/lib/python3.11/site-packages/numpy/core/include"],
)