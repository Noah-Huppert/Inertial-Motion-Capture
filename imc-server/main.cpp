#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

/* Socket */
int socket_fd = -1;
int socket_port = 1234;

struct sockaddr_in socket_address, client_address;
const int socket_connection_backlog_size = 10;

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

        if(strcmp(receive_buffer, "EXIT") == 10) {
            break;
        }

        // Send response
        send(client_socket_fd, receive_buffer, receive_buffer_size - 1, 0);
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