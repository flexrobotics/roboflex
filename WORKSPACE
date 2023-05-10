workspace(name = "roboflex")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# For now, use this fork of flatbuffers, which allows non-copying flex Blobs
http_archive(
    name = "com_github_google_flatbuffers",
    sha256 = "b73b56e4223357d92877b8c9f7db9a5d921d02e0ca9d2e5b7655929737cfc87a",
    strip_prefix = "flatbuffers-7efdc6237cbd595df672d83ab040415954d11e73",
    url = "https://github.com/colinator/flatbuffers/archive/7efdc6237cbd595df672d83ab040415954d11e73.zip",
)

# Contains a build file for eigen
http_archive(
    name = "cartographer",
    sha256 = "0539b025d43af4614471695e543f94f46c9a3914d8cef76d726406fd362c6650",
    strip_prefix = "cartographer-2.0.0",
    url = "https://github.com/cartographer-project/cartographer/archive/2.0.0.zip",
)

http_archive(
    name = "eigen",
    build_file = "@cartographer//bazel/third_party:eigen.BUILD",
    sha256 = "1ccaabbfe870f60af3d6a519c53e09f3dcf630207321dffa553564a8e75c4fc8",
    strip_prefix = "eigen-3.4.0",
    url = "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip",
)

http_archive(
    name = "xtl",
    build_file = "//third_party:xtl.BUILD",
    sha256 = "9e8e7dcc525500a4543226fe472a46a30636ee35274e4e30099071c5cbd4d05c",
    strip_prefix = "xtl-0.7.5",
    url = "https://github.com/xtensor-stack/xtl/archive/0.7.5.zip",
)

http_archive(
    name = "xsimd",
    build_file = "//third_party:xsimd.BUILD",
    sha256 = "5d362ec26c6f2332d1a2858891d770f5c0077133a81f885658f48c910a03fc90",
    strip_prefix = "xsimd-11.0.0",
    url = "https://github.com/xtensor-stack/xsimd/archive/11.0.0.zip",
)

http_archive(
    name = "xtensor",
    build_file = "//third_party:xtensor.BUILD",
    sha256 = "8cd062cfefce0f5bff8961e0c42b3636d5e7eddbe33e5c3167e3fb6d8380f50e",
    strip_prefix = "xtensor-0.24.6",
    url = "https://github.com/xtensor-stack/xtensor/archive/0.24.6.zip",
)
