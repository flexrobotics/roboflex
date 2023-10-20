load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])


#-----------------------------------------------#
# Third party libraries installed on the system #
#-----------------------------------------------#

# This is only used on the mac. For now, on linux, it doesn't 
# seem to be required. I'm not sure why, tbh. This needs a better solution.
cc_library(
    name = "numpy_headers",
    srcs = glob(["opt/homebrew/lib/python3.11/site-packages/numpy/core/include/numpy/*.h"]),
    includes = ["opt/homebrew/lib/python3.11/site-packages/numpy/core/include"],
)