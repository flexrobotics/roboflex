workspace(name = "roboflex")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


#------------------#
# System workspace #
#------------------#

# Hack to allow access to system libraries
new_local_repository(
    name = "system",
    build_file = "third_party/system.BUILD",
    path = "/",
)


#---------------------#
# Remote repositories #
#---------------------#

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

http_archive(
    name = "cppzmq",
    build_file = "//third_party:cppzmq.BUILD",
    sha256 = "32d07788bd60ba3cef09d43c7906153325c950c2dc4a9913876e4327dadca121",
    strip_prefix = "cppzmq-4.6.0",
    url = "https://github.com/zeromq/cppzmq/archive/v4.6.0.zip",
)

http_archive(
    name = "xtensor_python",
    build_file = "//third_party:xtensor_python.BUILD",
    sha256 = "72a29daba0844b251296e99bb616f8dfe16b32d6f416fba22245ffd51eb6f4a4",
    strip_prefix = "xtensor-python-0.26.1",
    url = "https://github.com/xtensor-stack/xtensor-python/archive/0.26.1.zip",
)

http_archive(
    name = "rules_python",
    sha256 = "b6d46438523a3ec0f3cead544190ee13223a52f6a6765a29eae7b7cc24cc83a0",
    #strip_prefix = "rules_python-a0fbf98d4e3a232144df4d0d80b577c7a693b570",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.1.0/rules_python-0.1.0.tar.gz",
)

http_archive(
  name = "pybind11_bazel",
  strip_prefix = "pybind11_bazel-fc56ce8a8b51e3dd941139d329b63ccfea1d304b",
  sha256 = "6426567481ee345eb48661e7db86adc053881cb4dd39fbf527c8986316b682b9",
  urls = ["https://github.com/pybind/pybind11_bazel/archive/fc56ce8a8b51e3dd941139d329b63ccfea1d304b.zip"],
)

http_archive(
  name = "pybind11",
  sha256 = "832e2f309c57da9c1e6d4542dedd34b24e4192ecb4d62f6f4866a737454c9970",
  build_file = "@pybind11_bazel//:pybind11.BUILD",
  strip_prefix = "pybind11-2.10.4",
  urls = ["https://github.com/pybind/pybind11/archive/v2.10.4.tar.gz"],
)

# configure python for pybind11

load("@pybind11_bazel//:python_configure.bzl", "python_configure")
python_configure(
    name = "local_config_python", 
    python_version = "3",
)