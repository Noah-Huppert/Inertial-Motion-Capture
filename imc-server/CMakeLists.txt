cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Global
# -- Include
include(ExternalProject)

# WebSocket++
# -- Include
externalProject_add(websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG 0.6.0)

set(SOURCE_FILES main.cpp)
add_executable(imc-server ${SOURCE_FILES})
