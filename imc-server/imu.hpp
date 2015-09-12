#ifndef IMU_H
#define IMU_H

#include <iostream>
#include <chrono>

extern "C" {
#include "bno055.h"
};

#include "mraa.hpp"

#include "status.h"

#include "quaternion.hpp"
#include "vector3.hpp"

class IMU {
public:
    Quaternion rotation;
    Vector3 position;

    IMU();
    ~IMU();

    int start();
    int stop();

    int update_rotation();
    int update_position();
    int update();

private:
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