#include "imu.hpp"
#include <fstream>

/* MRAA */
mraa::I2c *i2c = new mraa::I2c(1);
const int i2c_buffer_length = 8;

s8 bno055_driver_i2c_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 bno055_driver_i2c_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void bno055_driver_delay(u32 msek);

/* IMU Class specific functions */
int IMU::start() {
    if(!bno055_driver_bound) {
        bno055_driver_bind();

        bno055_driver_bound = true;
    }

    if(!bno055_initialized) {
        s8 bno055_init_result = bno055_init(&bno055);

        if((int) bno055_init_result != 0) {
            std::cerr << TAG_ERROR << "Failed to bno055_init => " << bno055_init_result << std::endl;
            return IMC_FAIL;
        }

        bno055_initialized = true;
    }

    if(!bno055_power_mode_normal) {
        int bno055_power_mode_result = bno055_set_power_mode(POWER_MODE_NORMAL);

        if(bno055_power_mode_result != 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 power mode to normal" << std::endl;
            return IMC_FAIL;
        }

        bno055_power_mode_normal = true;
    }

    if(!bno055_operation_mode_ndof) {
        int bno055_set_operation_mode_result = bno055_set_operation_mode(OPERATION_MODE_NDOF);

        if(bno055_set_operation_mode_result != 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 operation mode to ndof" << std::endl;
            return IMC_FAIL;
        }

        bno055_operation_mode_ndof = true;
    }

    if(!bno055_accelerometer_sensitivity) {
        int bno055_set_accelerometer_sensitivity_result = bno055_set_accel_range(ACCEL_RANGE_16G);

        if(bno055_set_accelerometer_sensitivity_result != 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 accelerometer sensitivity" << std::endl;
            return IMC_FAIL;
        }
    }

    return IMC_SUCCESS;
}

int IMU::stop() {
    if(bno055_power_mode_normal) {
        int bno055_set_power_mode_result = bno055_set_power_mode(POWER_MODE_SUSPEND);

        if(bno055_set_power_mode_result != 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 power mode to suspend" << std::endl;
            return IMC_FAIL;
        }

        bno055_power_mode_normal = false;
    }

    return IMC_SUCCESS;
}

bool IMU::is_ready() {
    return bno055_initialized &&
           bno055_power_mode_normal &&
           bno055_operation_mode_ndof;
}

int IMU::update_rotation() {
    if(!is_ready()) {
        std::cerr << TAG_ERROR << "Failed to update rotation, bno055 not ready" << std::endl;
        return IMC_FAIL;
    }

    struct bno055_euler_double_t bno055_rotation;

    int bno055_read_rotation_result =  bno055_convert_double_euler_hpr_deg(&bno055_rotation);//Heading Pitch Roll

    if(bno055_read_rotation_result != SUCCESS) {
        std::cerr << TAG_ERROR << "Failed to read bno055 rotation" << std::endl;
        return IMC_FAIL;
    }

    rotation_lock.lock();
    rotation.x = bno055_rotation.p;
    rotation.y = bno055_rotation.h;
    rotation.z = bno055_rotation.r;
    rotation_lock.unlock();

    return IMC_SUCCESS;
}

/* bno055 Driver functions */
void IMU::bno055_driver_bind() {
    bno055.bus_write = bno055_driver_i2c_write;
    bno055.bus_read = bno055_driver_i2c_read;
    bno055.delay_msec = bno055_driver_delay;
    bno055.dev_addr = BNO055_I2C_ADDR2;
}

s8 bno055_driver_i2c_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) {
    i2c->address(dev_addr);

    u8 write_buffer[cnt + 1];

    write_buffer[0] = reg_addr;

    for(int i = 0; i < cnt; i++) {
        write_buffer[i + 1] = reg_data[i];
    }

    int write_status = i2c->write(write_buffer, cnt + 1);

    if(write_status == MRAA_SUCCESS) {
        return SUCCESS;
    } else {
        return ERROR;
    }
}

s8 bno055_driver_i2c_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) {
    i2c->address(dev_addr);
    i2c->writeByte(reg_addr);

    i2c->address(dev_addr);
    int bytes_read = i2c->read(reg_data, cnt);

    if(bytes_read > 0) {
        return SUCCESS;
    } else {
        return ERROR;
    }
}

void bno055_driver_delay(u32 msek) {
    usleep(msek / 1000);
}