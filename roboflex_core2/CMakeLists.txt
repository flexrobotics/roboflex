cmake_minimum_required(VERSION 3.22)

project(roboflex_core VERSION 0.1.0 DESCRIPTION "roboflex core")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

# Fetch the flatbuffers library from GitHub
FetchContent_Declare(
    flatbuffers
    GIT_REPOSITORY https://github.com/colinator/flatbuffers.git
)
FetchContent_MakeAvailable(flatbuffers)

# Download and configure xtl
FetchContent_Declare(xtl
    URL https://github.com/xtensor-stack/xtl/archive/0.7.5.zip
    URL_HASH SHA256=9e8e7dcc525500a4543226fe472a46a30636ee35274e4e30099071c5cbd4d05c
    DOWNLOAD_NO_PROGRESS ON
)
FetchContent_MakeAvailable(xtl)

# Download and configure xsimd
FetchContent_Declare(xsimd
    URL https://github.com/xtensor-stack/xsimd/archive/11.0.0.zip
    URL_HASH SHA256=5d362ec26c6f2332d1a2858891d770f5c0077133a81f885658f48c910a03fc90
    DOWNLOAD_NO_PROGRESS ON
    #OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(xsimd)

# Download and configure xtensor
FetchContent_Declare(xtensor
    URL https://github.com/xtensor-stack/xtensor/archive/0.24.6.zip
    URL_HASH SHA256=8cd062cfefce0f5bff8961e0c42b3636d5e7eddbe33e5c3167e3fb6d8380f50e
    DOWNLOAD_NO_PROGRESS ON
)
FetchContent_MakeAvailable(xtensor)

# Download and configure eigen
FetchContent_Declare(eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
    URL_HASH SHA256=1ccaabbfe870f60af3d6a519c53e09f3dcf630207321dffa553564a8e75c4fc8
    DOWNLOAD_NO_PROGRESS ON
)
FetchContent_MakeAvailable(eigen)

add_library(roboflex_core SHARED
    node.cpp
    message.cpp
    message_backing_store.cpp
    serialization/flex_tensor_format.cpp
    serialization/flex_utils.cpp
    core_nodes/frequency_generator.cpp 
    core_nodes/graph_control.cpp 
    core_nodes/metrics.cpp
    util/get_process_memory_usage.cpp
    util/utils.cpp
    core.h
    message.h
    node.h
    message_backing_store.h
    core_messages/core_messages.h
    core_nodes/callback_fun.h 
    core_nodes/filter_fun.h
    core_nodes/frequency_generator.h
    core_nodes/graph_control.h
    core_nodes/last_one.h
    core_nodes/map_fun.h
    core_nodes/message_printer.h
    core_nodes/metrics.h
    serialization/flex_eigen.h
    serialization/flex_tensor_format.h
    serialization/flex_utils.h
    serialization/flex_xtensor.h
    util/get_process_memory_usage.h
    util/utils.h
    util/uuid.h
)

set_property(TARGET roboflex_core PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(roboflex_core PROPERTIES 
    VERSION ${PROJECT_VERSION}
)

target_link_libraries(roboflex_core PRIVATE flatbuffers xtensor xsimd xtl)

# we want to include files like #include "roboflex/core/util/utils.h", meaning
# we WANT that "core" prefix. So do this:
target_include_directories(roboflex_core PUBLIC ..)



# make the examples
add_executable(tensors_0_cpp examples/cpp/tensors_0.cpp)
target_include_directories(tensors_0_cpp PUBLIC ..)
target_link_libraries(tensors_0_cpp PRIVATE roboflex_core flatbuffers xtensor xsimd xtl)

add_executable(basic_0_cpp examples/cpp/basic_0.cpp)
target_include_directories(basic_0_cpp PUBLIC ..)
target_link_libraries(basic_0_cpp PRIVATE roboflex_core flatbuffers xtensor xsimd xtl)
