#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "bno055.h"
#include "mraa.hpp"

/* Socket */
int socket_fd = -1;
int socket_port = 1234;

struct sockaddr_in socket_address, client_address;
const int socket_connection_backlog_size = 10;

/* I2C */
mraa::I2c *i2c(0);
int i2c_buffer_length = 8;

/* BNO055 */
s8 BNO055_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 BNO055_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 I2C_routine();

void BNO055_delay_msek(u32 msek);
s32 bno055_read_data();

struct bno055_t bno055;

s32 bno055_read_data() {
    I2C_routine();

    // Init bno055
    s32 comres = bno055_init(&bno055);
    comres += bno055_set_power_mode(POWER_MODE_NORMAL);

    comres += bno055_set_operation_mode(OPERATION_MODE_NDOF);

    // Rotation
    struct bno055_quaternion_t rotation_quaternion;

    comres += bno055_read_quaternion_wxyz(&rotation_quaternion);

    // Linear acceleration
    struct bno055_linear_accel_t raw_linear_acceleration;
    struct bno055_linear_accel_double_t linear_acceleration;

    comres += bno055_read_linear_accel_xyz(&raw_linear_acceleration);
    comres += bno055_convert_double_linear_accel_xyz_msq(&linear_acceleration);

    // De-Init bno055
    comres += bno055_set_power_mode(POWER_MODE_SUSPEND);

    return comres;
}

s8 I2C_routine() {
    bno055.bus_write = BNO055_I2C_bus_write;
    bno055.bus_read = BNO055_I2C_bus_read;
    bno055.delay_msec = BNO055_delay_msek;
    bno055.dev_addr = BNO055_I2C_ADDR1;

    return BNO055_ZERO_U8X;
}

s8 BNO055_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) {
    u8 array[i2c_buffer_length];
    u8 stringpos = BNO055_ZERO_U8X;
    array[BNO055_ZERO_U8X;] = reg_addr;
    for (stringpos = BNO055_ZERO_U8X; stringpos < cnt; stringpos++) {
        array[stringpos + BNO055_ONE_U8X] = *(reg_data + stringpos);
    }

    i2c->address(dev_addr);
    int i2c_write_result = i2c->write(array, cnt + 2);

    if(i2c_write_result == MRAA_SUCCESS) {
        return (s8) SUCCESS;
    } else {
        return (s8) ERROR;
    }
}

s8 BNO055_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) {
    u8 array[i2c_buffer_length] = {BNO055_ZERO_U8X;};
    u8 stringpos = BNO055_ZERO_U8X;
    array[BNO055_ZERO_U8X;] = reg_addr;

    i2c->address(dev_addr);
    int i2c_bytes_read = i2c->read(array, cnt);

    for (stringpos = BNO055_ZERO_U8X; stringpos < cnt; stringpos++) {
        *(reg_data + stringpos) = array[stringpos];
    }

    if(i2c_bytes_read > 0) {
        return (s8) SUCCESS;
    } else {
        return (s8) ERROR;
    }
}

void BNO055_delay_msek(u32 msek) {
    usleep(msek / 1000);
}

void start_server() {
    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        std::cerr << "Error creating socket(errno => " << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created socket(fd => " << socket_fd << ")" << std::endl;
    }

    // Tell socket to reuse address
    int resuse_addr = 1;
    int set_reuse_addr_result = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &resuse_addr, sizeof(resuse_addr));

    if (set_reuse_addr_result < 0) {
        std::cerr << "Failed to set \"SO_REUSEADDR\"(fd => " << socket_fd << ", errno => " << strerror(errno) << ")" << std::endl;
    }

    // Bind socket
    bzero((char *) &socket_address, sizeof(socket_address));

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(socket_port);

    int bind_result = bind(socket_fd, (struct sockaddr *) &socket_address, sizeof(socket_address));

    if (bind_result < 0) {
        std::cerr << "Error binding socket(errno => " << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Bound socket(port => " << socket_port << ")" << std::endl;
    }

    // Listen on address
    int listen_result = listen(socket_fd, socket_connection_backlog_size);

    if (listen_result < 0) {
        std::cerr << "Error listening on socket(errno => " << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Socket listening" << std::endl;
    }
}

void stop_server() {
    close(socket_fd);

    std::cout << "Closed socket(fd => " << socket_fd << ")" << std::endl;
}

void socket_accept() {

    // Accept connection
    bzero((char *) &client_address, sizeof(client_address));

    int client_len;
    int client_socket_fd = accept(socket_fd, (struct sockaddr *) &client_address, (socklen_t *) &client_len);

    if (client_socket_fd < 0) {
        std::cerr << "Failed to accept client connection(errno => " << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Accepted client socket connection(fd => " << client_socket_fd << ")" << std::endl;
    }

    while(true) {
        // Receive content
        const int receive_buffer_size = 256;
        char receive_buffer[receive_buffer_size];

        bzero(receive_buffer, receive_buffer_size - 1);

        int receive_result = recv(client_socket_fd, receive_buffer, receive_buffer_size - 1, 0);

        if (receive_result < 0) {
            std::cerr << "Failed to receive from client socket(fd => " << client_socket_fd << ", errno => " << strerror(errno) << ")"  << std::endl;
        } else {
            std::cout << "Received from client socket(fd => " << client_socket_fd << std::endl
            << "                     content => " << receive_buffer << ")" << std::endl;
        }

        // Send response
        send(client_socket_fd, receive_buffer, receive_buffer_size - 1, 0);

        if(strcmp(receive_buffer, "EXIT") == 0) {
            break;
        }
    }

    close(client_socket_fd);

    std::cout << "Closed client socket connection(fd => " << client_socket_fd << ")" << std::endl;
}



int main() {
    start_server();

    std::thread socket_accept_thread(socket_accept);
    socket_accept_thread.join();

    stop_server();

    return EXIT_SUCCESS;
}