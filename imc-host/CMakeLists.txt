cmake_minimum_required(VERSION 2.8.4)
project(imc-host)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Host
# -- lib
add_subdirectory(lib)

# -- src
add_subdirectory(src)

add_executable(imc-host ${SRC_FILES})
target_link_libraries(imc-host /usr/lib/libmraa.so)
target_link_libraries(imc-host bno055-driver)