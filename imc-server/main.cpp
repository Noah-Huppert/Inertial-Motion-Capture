//#include <libsocket/inetserverstream.hpp>
//#include <libsocket/exception.hpp>

#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

/* Socket */
int socket_fd = -1;
sockaddr_in socket_address, client_address;
const int socket_connection_backlog_size = 10;

void start_server() {
    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Created socket(fd => " << socket_fd << ")" << std::endl;
    }

    // Bind socket
    bzero((char *) &socket_address, sizeof(socket_address));

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(1234);

    int bind_result = bind(socket_fd, (struct sockaddr *) &socket_address, sizeof(socket_address));

    if (bind_result < 0) {
        std::cerr << "Error binding socket" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Bound socket(port => " << socket_address.sin_port << ")" << std::endl;
    }

    // Listen on address
    int listen_result = listen(socket_fd, socket_connection_backlog_size);

    if (listen_result < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Socket listening" << std::endl;
    }
}

void socket_accept() {
    int client_len;
    int client_socket_fd = accept(socket_fd, (struct sockaddr *) &client_address, (socklen_t * ) &client_len);

    if (client_socket_fd < 0) {
        std::cerr << "Failed to accept client connection" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Accepted socket connection(address => " << client_address.sin_addr.s_addr
                                                              << ", port => " << client_address.sin_port
                                                << ")" << std::endl;
    }

    const int receive_buffer_size = 256;
    char receive_buffer[receive_buffer_size];

    recv(client_socket_fd, receive_buffer, receive_buffer_size, 0);
}

int main() {
    start_server();

    std::thread socket_accept_thread(socket_accept);
    socket_accept_thread.join();

    return EXIT_SUCCESS;
}