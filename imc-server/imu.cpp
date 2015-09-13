#include "imu.hpp"

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
    if(!is_ready()) {
        std::cerr << TAG_ERROR << "Failed to update position, bno055 not ready" << std::endl;
        return IMC_FAIL;
    }

    // Get Linear Acceleration
    std::cout << TAG_DEBUG << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;

    position_lock.lock();
    // Integrate
    position_lock.unlock();

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
    u8 write_buffer[i2c_buffer_length];
    bzero(write_buffer,  8);

    write_buffer[0] = reg_addr;

    for(int i = 0; i < cnt; i ++) {
        write_buffer[i + 1] = *(reg_data + i);
    }

    i2c->address(dev_addr);
    int i2c_write_result = i2c->write(write_buffer, i2c_buffer_length);

    if(i2c_write_result == MRAA_SUCCESS) {
        return (s8) SUCCESS;
    } else {
        return (s8) ERROR;
    }
/*
    u8 array[i2c_buffer_length];
    u8 stringpos = BNO055_ZERO_U8X;
    array[BNO055_ZERO_U8X] = reg_addr;
    for (stringpos = BNO055_ZERO_U8X; stringpos < cnt; stringpos++) {
        array[stringpos + BNO055_ONE_U8X] = *(reg_data + stringpos);
    }

    i2c->address(dev_addr);
    int i2c_write_result = i2c->write(array, i2c_buffer_length);

    if(i2c_write_result == MRAA_SUCCESS) {
        std::cout << "WRITE SUCCESS" << std::endl;
        return (s8) SUCCESS;
    } else {
        std::cout << "WRITE FAIL " << i2c_write_result << std::endl;
        return (s8) ERROR;
    }
*/
}

s8 bno055_driver_i2c_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) {
    u8 read_buffer[i2c_buffer_length];

    read_buffer[0] = reg_addr;

    i2c->address(dev_addr);
    int bytes_read = i2c->read(read_buffer, (int) cnt);

    for(int i = 0; i < i2c_buffer_length; i++) {
        *(reg_data + i) = read_buffer[i];
    }

    if(bytes_read > 0) {
        return (s8) SUCCESS;
    } else {
        return (s8) ERROR;
    }
    /*
    u8 array[i2c_buffer_length];

    u8 stringpos = BNO055_ZERO_U8X;
    array[BNO055_ZERO_U8X] = reg_addr;

    i2c->address(dev_addr);
    int i2c_bytes_read = i2c->read(array, (int) cnt);

    for (stringpos = BNO055_ZERO_U8X; stringpos < cnt; stringpos++) {
        *(reg_data + stringpos) = array[stringpos];
    }

    if(i2c_bytes_read > 0) {
        std::cout << "READ SUCCESS" << std::endl;
        return (s8) SUCCESS;
    } else {
        std::cout << "READ FAIL Bytes Read" << std::endl;
        return (s8) ERROR;
    }
     */
}

void bno055_driver_delay(u32 msek) {
    usleep(msek / 1000);
}