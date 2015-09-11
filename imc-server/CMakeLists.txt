cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Global
# -- Include
include(ExternalProject)

# BNO055
add_subdirectory(bno055)

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
add_dependencies(imc-server mraa)
target_link_libraries(imc-server libmraa.so)
target_link_libraries(imc-server bno055)