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

#include "difference_filter.hpp"
#include "csv_log.hpp"

class IMU {
public:
    std::mutex rotation_lock;
    Quaternion rotation;

    int df_size = 10;
    DifferenceFilter accel_df_x = DifferenceFilter(df_size);
    DifferenceFilter accel_df_y = DifferenceFilter(df_size);
    DifferenceFilter accel_df_z = DifferenceFilter(df_size);

    Vector3 last_accel;
    Vector3 last_vel;

    std::mutex position_lock;
    Vector3 position;

    IMU() {
        accel_df_z.difference_threshold = 0.25;
    };
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