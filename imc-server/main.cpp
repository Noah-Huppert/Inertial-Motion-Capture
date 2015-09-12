#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "imu.hpp"

/* Socket */
int socket_fd = -1;
int socket_port = 1234;

struct sockaddr_in socket_address, client_address;
const int socket_connection_backlog_size = 10;

IMU *imu;


/*
s32 bno055_read_data() {
    I2C_routine();

    // Init bno055-driver
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

    // De-Init bno055-driver
    comres += bno055_set_power_mode(POWER_MODE_SUSPEND);

    return comres;
}
*/

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

    socklen_t client_len = sizeof(client_address);
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
        } else if(strcmp(receive_buffer, "NEXT") == 0) {
            imu->update_rotation();
            std::cout << "rotation = " << imu->rotation.to_string() << std::endl;
        }
    }

    close(client_socket_fd);

    std::cout << "Closed client socket connection(fd => " << client_socket_fd << ")" << std::endl;
}

#include "mraa.h"
#include "mraa.hpp"

int main() {
    imu = new IMU();
    imu->start();

    start_server();

    std::thread socket_accept_thread(socket_accept);
    socket_accept_thread.join();

    stop_server();

    delete imu;

    /*
    mraa::I2c *i2c = new mraa::I2c(1);

    std::cout << "ADDRESS 0x01 = " << i2c->address(0x29) << std::endl;

    u8 write_buffer[8];
    bzero(write_buffer,  8);

    write_buffer[0] = 0x00;
    write_buffer[1] = 0xAF;
    write_buffer[2] = 0x00;
    write_buffer[3] = 0xAF;
    write_buffer[4] = 0x00;
    write_buffer[5] = 0xAF;
    write_buffer[6] = 0x00;
    write_buffer[7] = 0xAF;

    std::cout << "WRITE => " << i2c->write(write_buffer, 8) << std::endl;
    delete i2c;
    */


    return EXIT_SUCCESS;
}