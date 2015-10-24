#ifndef IMU_H
#define IMU_H

#include <iostream>
#include <chrono>
#include <mutex>
#include <cmath>
#include <deque>

extern "C" {
#include "bno055.h"
};

#include "mraa.hpp"

#include "status.h"
#include "log.h"

#include "quaternion.hpp"

class IMU {
public:
    std::mutex rotation_lock;
    Quaternion rotation;

    IMU() {};
    ~IMU() {};

    int start();
    int stop();

    int update_rotation();

private:
    long last_rotation_update_time = -1;

    bool bno055_driver_bound = false;
    bool bno055_initialized = false;
    bool bno055_power_mode_normal = false;
    bool bno055_operation_mode_ndof = false;

    /* BNO055 */
    struct bno055_t bno055;

    bool is_ready();

    /* bno055 Driver */
    void bno055_driver_bind();
};

#endif