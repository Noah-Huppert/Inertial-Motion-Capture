cmake_minimum_required(VERSION 2.8.4)
project(bno055)

add_library(bno055 ${CMAKE_CURRENT_SOURCE_DIR}/bno055.c)
target_include_directories(bno055 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})