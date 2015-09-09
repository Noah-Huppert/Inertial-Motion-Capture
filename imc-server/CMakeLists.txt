cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Global
# -- Include
include(ExternalProject)

# BNO055
# -- Include
include_directories(${CMAKE_SOURCE_DIR}/bno055)
set(SOURCE_FILES ${SOURCE_FILES}
        ${CMAKE_SOURCE_DIR}/bno055/bno055.c)

# -- Build
add_library(bno055 SHARED ${CMAKE_SOURCE_DIR}/bno055/bno055.c)

# MRAA
# -- Build
externalProject_add(mraa
        GIT_REPOSITORY https://github.com/intel-iot-devkit/mraa.git
        GIT_TAG        v0.7.5
)

# Compile
# -- Source
set(SOURCE_FILES ${SOURCE_FILES}
        main.cpp)

# -- Create
add_executable(imc-server ${SOURCE_FILES})
target_link_libraries(imc-server ${CMAKE_BINARY_DIR}/libbno055.so)
target_link_libraries(imc-server libmraa.so)