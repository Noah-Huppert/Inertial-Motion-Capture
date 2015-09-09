cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Global
# -- Include
include(ExternalProject)

# BNO055
# -- Configure
set(bno055_INSTALL_DIR ${CMAKE_BINARY_DIR}/lib)
# -- Build
externalProject_add(bno055
        GIT_REPOSITORY    https://github.com/BoschSensortec/BNO055_driver.git
        CONFIGURE_COMMAND :
        BUILD_COMMAND     :
        INSTALL_COMMAND   mkdir -p ${bno055_INSTALL_DIR} && cp -r ../bno055 ${bno055_INSTALL_DIR}
)

set(bno055_INSTALL_DIR ${bno055_INSTALL_DIR}/bno055)

# -- Include
include_directories(${bno055_INSTALL_DIR})

# -- Create
add_library(bno055-lib ${bno055_INSTALL_DIR}/bno055.c)

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
target_link_libraries(imc-server bno055-lib)