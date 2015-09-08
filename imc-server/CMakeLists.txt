cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Global
# -- Include
include(ExternalProject)

# BNO055
# -- Build
externalProject_add(bno055
        GIT_REPOSITORY    https://github.com/BoschSensortec/BNO055_driver.git
        CONFIGURE_COMMAND :
        BUILD_COMMAND     :
        INSTALL_COMMAND   :
)

# -- Include
ExternalProject_Get_Property(bno055 SOURCE_DIR)

include_directories("${SOURCE_DIR}")

# MRAA
# -- Build
externalProject_add(mraa
        GIT_REPOSITORY https://github.com/intel-iot-devkit/mraa.git
        GIT_TAG        v0.7.5
)

# Compile
# -- Source
set(SOURCE_FILES main.cpp)

# -- Create
add_executable(imc-server ${SOURCE_FILES})
