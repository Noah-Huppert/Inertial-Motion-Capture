cmake_minimum_required(VERSION 2.8.4)
project(bno055-driver)

add_library(bno055-driver ${CMAKE_CURRENT_SOURCE_DIR}/bno055.c)
target_include_directories(bno055-driver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})