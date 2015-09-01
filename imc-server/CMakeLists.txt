cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Global
# -- Include
include(ExternalProject)

# Simple-WebSocket-Server
# -- Include
externalProject_add(libsocket
    GIT_REPOSITORY https://github.com/dermesser/libsocket.git
    GIT_TAG v2.4.1)

# -- Link
ExternalProject_Get_Property(libsocket install_dir)
include_directories(${install_dir}/include)

# Compile
# -- Source
set(SOURCE_FILES main.cpp)

# -- Create
add_executable(imc-server ${SOURCE_FILES})

# -- Link
target_link_libraries(imc-server libsocket++.so)
