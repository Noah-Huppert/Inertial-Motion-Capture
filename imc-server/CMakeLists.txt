cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Compile
# -- Source
set(SOURCE_FILES main.cpp)

# -- Create
add_executable(imc-server ${SOURCE_FILES})
