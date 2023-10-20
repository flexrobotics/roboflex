# python/CMakeLists.txt
cmake_minimum_required(VERSION 3.25)

project(roboflex_core_python VERSION 0.1.1 DESCRIPTION "roboflex core python wrapping")

# If you don't do this, then the pybind11_add_module will think it's
# standalone and will not link correctly.
set(PYBIND11_CPP_STANDARD -std=c++20)


# -------------------- 
# Get the dependencies

include(FetchContent)

# download and build pybind11
FetchContent_Declare(pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.8.1
)
FetchContent_MakeAvailable(pybind11)
# Apparently we dont' need: add_subdirectory(pybind11)

# download and build xtensor_python
FetchContent_Declare(xtensor_python
    GIT_REPOSITORY https://github.com/xtensor-stack/xtensor-python.git
    GIT_TAG        0.26.1
)
FetchContent_MakeAvailable(xtensor_python)


# -------------------- 
# The roboflex core pybind library

# Use the pybind11 provided function to create a module.
pybind11_add_module(roboflex_core_python_ext
    pybindings.cpp
)

# Link against your core library and any necessary dependencies
target_link_libraries(roboflex_core_python_ext PRIVATE roboflex_core flatbuffers_util xtensor-python xtensor xtl xsimd eigen)

# If you have specific compile definitions or options for just the Python module
# target_compile_definitions(pyroboflex PRIVATE SOME_DEFINITION)

# You can set properties for the target if necessary.
set_target_properties(roboflex_core_python_ext PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)


# -------------------- 
# install

# Install the generated Python module to the desired destination.
# This installs the compiled module.
install(TARGETS roboflex_core_python_ext
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages/roboflex
)

# # Install the Python files
# NEIN!!! We're gonna let setup.py
# install these files. Is that the right thing to do?
# install(FILES
#     dynoflex.py
#     flexbuffers.py
#     flextensors.py
#     __init__.py
#     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages/roboflex
# )