#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "status.h"
#include "log.h"

class SocketServer {
public:
    static const bool cout_socket_receive = false;
    static const bool cout_socket_send = false;

    SocketServer(): server_socket_port(-1) {
        std::cerr << TAG_ERROR << "SocketServer default constructor used, port will not be set!" << std::endl;
    }

    SocketServer(int server_socket_port): server_socket_port(server_socket_port) {}
    ~SocketServer() {};

    int start();
    int stop();

    int accept_connection();

    virtual void handle_client_socket(int client_socket_fd) = 0;

    static int socket_receive(int socket_fd, char *receive_buffer, int receive_buffer_length);
    static int socket_send(int socket_fd, char *send_buffer, int send_buffer_length);

private:
    bool server_socket_started = false;

    const int server_socket_port;
    int server_socket_fd = -1;

    void server_socket_close();

    struct sockaddr_in server_socket_address;
};

#endif