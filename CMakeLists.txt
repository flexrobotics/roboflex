cmake_minimum_required(VERSION 3.18)

option(BUILD_ROBOFLEX_PYTHON_EXT "Build the Roboflex Python Extension" ON)

project(roboflex_core VERSION 0.1.35)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# -------------------- 
# Get the dependencies

find_package(Threads)

include(FetchContent)

# Download and configure xtl
FetchContent_Declare(xtl
    URL https://github.com/xtensor-stack/xtl/archive/0.7.5.zip
    URL_HASH SHA256=9e8e7dcc525500a4543226fe472a46a30636ee35274e4e30099071c5cbd4d05c
    DOWNLOAD_NO_PROGRESS ON
    DOWNLOAD_EXTRACT_TIMESTAMP 1
)
FetchContent_MakeAvailable(xtl)

# Download and configure xsimd
FetchContent_Declare(xsimd
    URL https://github.com/xtensor-stack/xsimd/archive/11.0.0.zip
    URL_HASH SHA256=5d362ec26c6f2332d1a2858891d770f5c0077133a81f885658f48c910a03fc90
    DOWNLOAD_NO_PROGRESS ON
    DOWNLOAD_EXTRACT_TIMESTAMP 1
)
FetchContent_MakeAvailable(xsimd)

# Download and configure xtensor
FetchContent_Declare(xtensor
    URL https://github.com/xtensor-stack/xtensor/archive/0.24.6.zip
    URL_HASH SHA256=8cd062cfefce0f5bff8961e0c42b3636d5e7eddbe33e5c3167e3fb6d8380f50e
    DOWNLOAD_NO_PROGRESS ON
    DOWNLOAD_EXTRACT_TIMESTAMP 1
)
set(XTENSOR_USE_XSIMD 1)
FetchContent_MakeAvailable(xtensor)

# Download and configure eigen
FetchContent_Declare(eigen
    # eigen 3.4.0 seems to have a problemo...
    # URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
    # URL_HASH SHA256=1ccaabbfe870f60af3d6a519c53e09f3dcf630207321dffa553564a8e75c4fc8
    # GIT_TAG        origin/master
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        877c2d1e9b08b9630a924ee2e06469048b124e2d 
    DOWNLOAD_NO_PROGRESS ON
    DOWNLOAD_EXTRACT_TIMESTAMP 1
)
FetchContent_MakeAvailable(eigen)

# Download and configure flatbuffers (although we only use flexbuffers)
FetchContent_Declare(flatbuffers
    GIT_REPOSITORY https://github.com/colinator/flatbuffers.git
    GIT_TAG origin/master
)

# We want to be able to use ${flatbuffers_SOURCE_DIR}
FetchContent_GetProperties(flatbuffers)
if(NOT flatbuffers_POPULATED)
    FetchContent_Populate(flatbuffers)
endif()

# So we create our own flatbuffers library target
add_library(flatbuffers_util INTERFACE)
target_include_directories(flatbuffers_util INTERFACE
    $<BUILD_INTERFACE:${flatbuffers_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


# -------------------- 
# The roboflex core library

add_library(roboflex_core STATIC

    # Source files
    src/core_nodes/graph_root.cpp
    src/core_nodes/frequency_generator.cpp
    src/core_nodes/metrics.cpp
    src/core_nodes/take.cpp
    src/core_nodes/universal_data_saver.cpp    
    src/core_nodes/universal_data_player.cpp
    src/message_backing_store.cpp
    src/message.cpp
    src/node.cpp
    src/serialization/flex_tensor_format.cpp
    src/serialization/flex_utils.cpp
    #src/serialization/serialization.cpp
    src/util/utils.cpp
    src/util/get_process_memory_usage.cpp
    
    # Header files (not strictly necessary for building, but can be useful for some IDEs)
    include/roboflex_core/core.h
    include/roboflex_core/core_messages/core_messages.h
    include/roboflex_core/core_nodes/null.h
    include/roboflex_core/core_nodes/callback_fun.h
    include/roboflex_core/core_nodes/core_nodes.h
    include/roboflex_core/core_nodes/every_n.h
    include/roboflex_core/core_nodes/filter_fun.h
    include/roboflex_core/core_nodes/frequency_generator.h
    include/roboflex_core/core_nodes/graph_root.h
    include/roboflex_core/core_nodes/last_one.h
    include/roboflex_core/core_nodes/map_fun.h
    include/roboflex_core/core_nodes/message_printer.h
    include/roboflex_core/core_nodes/metrics.h
    include/roboflex_core/core_nodes/producer.h
    include/roboflex_core/core_nodes/take.h
    include/roboflex_core/core_nodes/tensor_buffer.h
    include/roboflex_core/core_nodes/universal_data_saver.h
    include/roboflex_core/core_nodes/universal_data_player.h
    include/roboflex_core/message_backing_store.h
    include/roboflex_core/message.h
    include/roboflex_core/node.h
    include/roboflex_core/serialization/flex_eigen.h
    include/roboflex_core/serialization/flex_tensor_format.h
    include/roboflex_core/serialization/flex_utils.h
    include/roboflex_core/serialization/flex_xtensor.h
    #include/roboflex_core/serialization/serialization.h
    include/roboflex_core/serialization/serializer.h
    include/roboflex_core/util/event.h
    include/roboflex_core/util/utils.h
    include/roboflex_core/util/uuid.h
    include/roboflex_core/util/get_process_memory_usage.h
)

target_include_directories(roboflex_core PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/eigen3>
)

target_link_libraries(roboflex_core PUBLIC 
    flatbuffers_util 
    xtensor 
    xsimd 
    xtl 
    #xtensor::optimize 
    #xtensor::use_xsimd
    Eigen3::Eigen
    ${CMAKE_THREAD_LIBS_INIT}
)

set_property(TARGET roboflex_core PROPERTY 
    POSITION_INDEPENDENT_CODE ON
)

set_target_properties(roboflex_core PROPERTIES 
    VERSION ${PROJECT_VERSION}
)


# -------------------- 
# Examples

# basic_0 example
add_executable(basic_0_cpp examples/cpp/basic_0.cpp)
target_link_libraries(basic_0_cpp PRIVATE roboflex_core flatbuffers_util xtensor xsimd xtl eigen)

# tensors_0 example
add_executable(tensors_0_cpp examples/cpp/tensors_0.cpp)
target_link_libraries(tensors_0_cpp PRIVATE roboflex_core flatbuffers_util xtensor xsimd xtl eigen)

# tensor_buffer example
add_executable(tensor_buffer_0 examples/cpp/tensor_buffer_0.cpp)
target_link_libraries(tensor_buffer_0 PRIVATE roboflex_core flatbuffers_util xtensor xsimd xtl eigen)


# -------------------- 
# install

# install our flatbuffers utility library
install(TARGETS flatbuffers_util
    EXPORT roboflex_coreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# install all the files in flatbuffers/include to include (is that right?)
install(DIRECTORY ${flatbuffers_SOURCE_DIR}/include/ 
    DESTINATION include
)

# install the 'roboflex_core' library
install(TARGETS roboflex_core
    EXPORT roboflex_coreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# and roboflex_core public headers
install(DIRECTORY include/roboflex_core
    DESTINATION include
)

# export the targets to a make script
install(EXPORT roboflex_coreTargets
    FILE roboflex_coreTargets.cmake
    NAMESPACE roboflex_core:: # do we want this?
    DESTINATION lib/cmake/roboflex_core # do we want or need cmake/ ?
)

# generate actual config file 
# The generated file will include information 
# about where to find the installed headers, 
# libraries, and any other required files,
# INCLUDING DEPENDENCIES such as xtensor.
include(CMakePackageConfigHelpers)
configure_package_config_file(Config.cmake.in # template
    ${CMAKE_CURRENT_BINARY_DIR}/roboflex_coreConfig.cmake # output
    INSTALL_DESTINATION lib/cmake/roboflex_core # relative to CMAKE_INSTALL_PREFIX
)
# install the config file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/roboflex_coreConfig.cmake
    DESTINATION lib/cmake/roboflex_core
)

# install the examples
install(TARGETS tensors_0_cpp basic_0_cpp
    RUNTIME DESTINATION bin/examples
)


# --------------------
# build python bindings, only if we're building the core library.

if(BUILD_ROBOFLEX_PYTHON_EXT)
    message(">>>>>>>>>>>>> BUILDING ROBOFLEX CORE PYTHON BINDINGS <<<<<<<<<<<<<<<<<<<")
    add_subdirectory(python)
endif()
