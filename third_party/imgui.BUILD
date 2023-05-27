load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "imgui",
    linkopts = ["-ldl", "-lGL", "-L/usr/lib/x86_64-linux-gnu"],
    visibility = ["//visibility:public"],
    includes = ["."],
    hdrs = [
        "imgui.h",
        "imconfig.h",
        "imgui_internal.h",
        "imstb_textedit.h",
        "imstb_rectpack.h",
        "imstb_truetype.h",
        "backends/imgui_impl_sdl2.h",
        "backends/imgui_impl_opengl3.h",
        "backends/imgui_impl_opengl3_loader.h",
    ],
    srcs = [
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imgui_demo.cpp",
        "backends/imgui_impl_sdl2.cpp",
        "backends/imgui_impl_opengl3.cpp",
    ],
    deps = [
        "@roboflex//third_party:sdl2",
        "@roboflex//third_party:glew",
    ]
)
