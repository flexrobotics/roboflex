cmake_minimum_required(VERSION 3.5)

project(dynamixel_sdk)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(dynamixel_sdk
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/packet_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/protocol1_packet_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/protocol2_packet_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/group_sync_read.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/group_sync_write.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/group_bulk_read.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/group_bulk_write.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/port_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/src/dynamixel_sdk/port_handler_linux.cpp
)

target_include_directories(dynamixel_sdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dynamixel_sdk/c++/include/dynamixel_sdk)
