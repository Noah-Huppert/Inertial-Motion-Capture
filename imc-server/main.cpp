#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>

#include "socket_server.hpp"
#include "imu.hpp"
#include "imc_time.hpp"

std::mutex imu_lock;
IMU *imu;
long imu_update_interval = 5;

std::mutex server_running_lock;
bool server_running = true;

class IMUSocketServer: public SocketServer {
public:
    IMUSocketServer(): SocketServer() {}
    IMUSocketServer(int port): SocketServer(port) {}

    void handle_client_socket(int client_socket_fd) {
        int buffer_size = 256;
        char receive_buffer[buffer_size];
        char send_buffer[buffer_size];

        while(true) {
            SocketServer::socket_receive(client_socket_fd, receive_buffer, buffer_size);

            bzero(send_buffer, buffer_size);

            if(strcmp(receive_buffer, "KILL") == 0) {
                server_running_lock.lock();
                server_running = false;
                server_running_lock.unlock();
                break;
            } else if(strcmp(receive_buffer, "EXIT") == 0) {
                break;
            } else if(strcmp(receive_buffer, "RESET") == 0) {
                imu_lock.lock();

                imu->position_lock.lock();
                imu->position = Vector3();
                imu->vel_sum = Vector3();
                imu->last_vel = Vector3();
                imu->last_accel = Vector3();
                imu->position_lock.unlock();

                imu->rotation_lock.lock();
                imu->rotation = Quaternion();
                imu->rotation_lock.unlock();

                imu_lock.unlock();
            } else if(strcmp(receive_buffer, "NEXT") == 0) {
                /*
                 * Send data in the following format
                 * position.x position.y position.z rotation.w rotation.x rotation.y rotation.z
                 */

                std::stringstream out_stream;

                imu_lock.lock();

                imu->position_lock.lock();
                out_stream << imu->position.to_space_delimited() << " ";
                imu->position_lock.unlock();

                imu->rotation_lock.lock();
                out_stream << imu->rotation.to_space_delimited();
                imu->rotation_lock.unlock();

                imu_lock.unlock();

                std::string temp_out = out_stream.str();
                strncpy(send_buffer, temp_out.c_str(), buffer_size);
            }
            SocketServer::socket_send(client_socket_fd, send_buffer, buffer_size);
        }
    }
};

IMUSocketServer *socket_server;

void socket_server_accept() {
    while(true) {
        socket_server->accept_connection();

        server_running_lock.lock();
        if (!server_running) {
            server_running_lock.unlock();
            break;
        }
        server_running_lock.unlock();
    }

    server_running_lock.lock();
    server_running = false;
    server_running_lock.unlock();
}

void imu_update() {
    long last_update = imc_time();

    while(true) {
        if(imc_time() - last_update >= imu_update_interval) {
            server_running_lock.lock();
            if (!server_running) {
                break;
            }
            server_running_lock.unlock();

            imu_lock.lock();
            imu->update();
            imu_lock.unlock();

            last_update = imc_time();
        }
    }
}

int main() {
    // Create Objects
    imu_lock.lock();
    imu = new IMU();
    imu_lock.unlock();

    socket_server = new IMUSocketServer(1234);

    // Start Objects
    imu_lock.lock();
    imu->start();
    imu_lock.unlock();

    socket_server->start();

    // Create Threads
    std::thread socket_server_accept_thread(socket_server_accept);
    std::thread imu_update_thread(imu_update);

    // Join Threads
    socket_server_accept_thread.join();
    imu_update_thread.join();

    // Stop Objects
    socket_server->stop();

    imu_lock.lock();
    imu->stop();
    imu_lock.unlock();


    // Delete Objects
    delete socket_server;
    delete imu;

    return EXIT_SUCCESS;
}