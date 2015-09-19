#include "imu.hpp"
#include <fstream>

CSVLog csv_log("integration_log.csv");
long csv_log_start_time = 0;

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

        if((int) bno055_init_result < 0) {
            std::cerr << TAG_ERROR << "Failed to bno055_init => " << bno055_init_result << std::endl;
            return IMC_FAIL;
        }

        bno055_initialized = true;
    }

    if(!bno055_power_mode_normal) {
        int bno055_power_mode_result = bno055_set_power_mode(POWER_MODE_NORMAL);

        if(bno055_power_mode_result < 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 power mode to normal" << std::endl;
            return IMC_FAIL;
        }

        bno055_power_mode_normal = true;
    }

    if(!bno055_operation_mode_ndof) {
        int bno055_set_operation_mode_result = bno055_set_operation_mode(OPERATION_MODE_NDOF);

        if(bno055_set_operation_mode_result < 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 operation mode to ndof" << std::endl;
            return IMC_FAIL;
        }

        bno055_operation_mode_ndof = true;
    }

    if(!bno055_accelerometer_sensitivity) {
        int bno055_set_accelerometer_sensitivity_result = bno055_set_accel_range(ACCEL_RANGE_16G);

        if(bno055_set_accelerometer_sensitivity_result < 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 accelerometer sensitivity" << std::endl;
            return IMC_FAIL;
        }
    }

    csv_log.add_columns({
            "accel-center-x",
            "accel-center-y",
            "accel-center-z",

            "accel-x",
            "accel-y",
            "accel-z",

            "accel-df-x",
            "accel-df-y",
            "accel-df-z",

            "pos-x",
            "pos-y",
            "pos-z",

            "t"
    });

    csv_log.lock_columns();

    csv_log.open();

    csv_log_start_time = imc_time();

    return IMC_SUCCESS;
}

int IMU::stop() {
    if(bno055_power_mode_normal) {
        int bno055_set_power_mode_result = bno055_set_power_mode(POWER_MODE_SUSPEND);

        if(bno055_set_power_mode_result < 0) {
            std::cerr << TAG_ERROR << "Failed to set bno055 power mode to suspend" << std::endl;
            return IMC_FAIL;
        }

        bno055_power_mode_normal = false;
    }

    csv_log.close();

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

    struct bno055_quaternion_t rotation_quaternion;

    int bno055_read_quaternion_result =  bno055_read_quaternion_wxyz(&rotation_quaternion);

    if(bno055_read_quaternion_result != SUCCESS) {
        std::cerr << TAG_ERROR << "Failed to read bno055 quaternion" << std::endl;
        return IMC_FAIL;
    }

    rotation_lock.lock();
    rotation.w = rotation_quaternion.w;
    rotation.x = rotation_quaternion.x;
    rotation.y = rotation_quaternion.y;
    rotation.z = rotation_quaternion.z;
    rotation_lock.unlock();

    return IMC_SUCCESS;
}

int IMU::update_position() {
    if(last_position_update_time == -1) {
        last_position_update_time = imc_time();
    }
    if(!is_ready()) {
        std::cerr << TAG_ERROR << "Failed to update position, bno055 not ready" << std::endl;
        return IMC_FAIL;
    }

    // Get Linear Acceleration
    struct bno055_linear_accel_double_t linear_acceleration;

    int get_linear_acceleration_result = bno055_convert_double_linear_accel_xyz_msq(&linear_acceleration);

    csv_log.add_to_line("accel-x", linear_acceleration.x);
    csv_log.add_to_line("accel-y", linear_acceleration.y);
    csv_log.add_to_line("accel-z", linear_acceleration.z);

    double accel_filtered_x = accel_df_x.value(linear_acceleration.x);
    double accel_filtered_y = accel_df_y.value(linear_acceleration.y);
    double accel_filtered_z = accel_df_z.value(linear_acceleration.z);

    csv_log.add_to_line("accel-center-x", accel_df_x.center_value_maf.average());
    csv_log.add_to_line("accel-center-y", accel_df_y.center_value_maf.average());
    csv_log.add_to_line("accel-center-z", accel_df_z.center_value_maf.average());

    csv_log.add_to_line("accel-df-x", accel_filtered_x);
    csv_log.add_to_line("accel-df-y", accel_filtered_y);
    csv_log.add_to_line("accel-df-z", accel_filtered_z);

    double side_x = (accel_filtered_x + last_accel.x) / 2;
    double side_y = (accel_filtered_y + last_accel.y) / 2;
    double side_z = (accel_filtered_z + last_accel.z) / 2;

    last_accel.x = accel_filtered_x;
    last_accel.y = accel_filtered_y;
    last_accel.z = accel_filtered_z;

    if(get_linear_acceleration_result < 0) {
        std::cout << TAG_ERROR << "Failed to get converted linear accleration" << std::endl;
        return IMC_FAIL;
    }

    // Integrate
    long delta_time = imc_time() - last_position_update_time;

    position_lock.lock();

    position.x += side_x * delta_time;
    position.y += side_y * delta_time;
    position.z += side_z * delta_time;

    csv_log.add_to_line("pos-x", position.x);
    csv_log.add_to_line("pos-y", position.y);
    csv_log.add_to_line("pos-z", position.z);

    position_lock.unlock();

    last_position_update_time = imc_time();

    csv_log.add_to_line("t", imc_time() - csv_log_start_time);

    csv_log.finish_line();

    return IMC_SUCCESS;
}

int IMU::update() {
    int update_rotation_result = update_rotation();
    int update_position_result = update_position();

    if(update_rotation_result != IMC_SUCCESS || update_position_result != IMC_SUCCESS) {
        return IMC_FAIL;
    }

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