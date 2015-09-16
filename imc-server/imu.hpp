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
#include "imc_time.hpp"

#include "quaternion.hpp"
#include "vector3.hpp"

#include "moving_average_filter.hpp"

class IMU {
public:
    std::mutex rotation_lock;
    Quaternion rotation;

    int maf_size = 5;
    MovingAverageFilter accel_x_maf = MovingAverageFilter(maf_size);
    MovingAverageFilter accel_y_maf = MovingAverageFilter(maf_size);
    MovingAverageFilter accel_z_maf = MovingAverageFilter(maf_size);

    std::mutex position_lock;
    Vector3 position;

    IMU() {};
    ~IMU() {};

    int start();
    int stop();

    int update_rotation();
    int update_position();
    int update();

private:
    long last_position_update_time = -1;

    bool bno055_driver_bound = false;
    bool bno055_initialized = false;
    bool bno055_power_mode_normal = false;
    bool bno055_operation_mode_ndof = false;
    bool bno055_accelerometer_sensitivity = false;

    /* BNO055 */
    struct bno055_t bno055;

    bool is_ready();

    /* bno055 Driver */
    void bno055_driver_bind();
};

#endif