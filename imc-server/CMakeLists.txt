cmake_minimum_required(VERSION 2.8.4)
project(imc-server)

# CMake
# -- Config
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Global
# -- Include
include(ExternalProject)

# BNO055
add_subdirectory(bno055-driver)


# Compile
# -- Source
set(SOURCE_FILES ${SOURCE_FILES}
        status.h
        log.h

        csv_log.hpp
        csv_log.cpp

        moving_average_filter.hpp
        moving_average_filter.cpp

        difference_filter.hpp
        difference_filter.cpp

        imc_time.hpp
        imc_time.cpp

        quaternion.hpp
        vector3.hpp

        socket_server.hpp
        socket_server.cpp

        imu.hpp
        imu.cpp

        main.cpp)

# -- Create
add_executable(imc-server ${SOURCE_FILES})
target_link_libraries(imc-server /usr/lib/libmraa.so)
target_link_libraries(imc-server bno055-driver)