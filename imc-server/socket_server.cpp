#include "socket_server.hpp"

void SocketServer::server_socket_close() {
    if(server_socket_fd > 0) {
        int close_result = close(server_socket_fd);

        if(close_result < 0) {
            std::cerr << TAG_ERROR << "Error closing server socket(fd => " << server_socket_fd <<
                                                           ", errno => " << strerror(errno) <<
                                                            ")" << std::endl;
        } else {
            std::cout << TAG_INFO << "Server socket closed(fd => " << server_socket_fd << ")" << std::endl;
            server_socket_fd = -1;
            server_socket_started = false;
        }
    }
}

int SocketServer::start() {
    if(!server_socket_started) {
        // Create socket
        server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

        if(server_socket_fd < 0) {
            std::cerr << TAG_ERROR <<  "Error creating socket(errno => " << strerror(errno) << ")" << std::endl;
            return IMC_FAIL;
        } else {
            std::cout << TAG_INFO << "Server socket created(fd => " << server_socket_fd << ")" << std::endl;
        }

        // Set socket to re-use address
        int server_socket_reuse_address = 1;
        int set_reuse_address_result = setsockopt(server_socket_fd,
                                                  SOL_SOCKET,
                                                  SO_REUSEADDR,
                                                  (char *) &server_socket_reuse_address,
                                                  sizeof(server_socket_reuse_address)
        );

        if(set_reuse_address_result < 0) {
            std::cerr << TAG_ERROR << "Error setting \"SO_REUSEADDR\" for server socket(fd => " << server_socket_fd <<
                                                                                    ", errno => " << strerror(errno) <<
                                                                                     ")" << std::endl;
            server_socket_close();
            return IMC_FAIL;
        }

        // Bind socket
        memset(&server_socket_address, 0, sizeof(server_socket_address));

        server_socket_address.sin_family = AF_INET;
        server_socket_address.sin_addr.s_addr = INADDR_ANY;
        server_socket_address.sin_port = htons(server_socket_port);

        int bind_result = bind(server_socket_fd, (struct sockaddr *) &server_socket_address, sizeof(server_socket_address));

        if(bind_result < 0) {
            std::cerr << TAG_ERROR << "Error binding server socket(fd => " << server_socket_fd <<
                                                            ", errno => " << strerror(errno) <<
                                                            ")" << std::endl;

            server_socket_close();
            return IMC_FAIL;
        } else {
            std::cout << TAG_INFO << "Server socket bound(fd => " << server_socket_fd <<
                                                         ", port => " << server_socket_port <<
                                                         ")" << std::endl;
        }

        // Listen
        int listen_result = listen(server_socket_fd, 1);

        if(listen_result < 0) {
            std::cerr << TAG_ERROR << "Error listening on server socket(fd => " << server_socket_fd <<
                                                                       ", errno => " << strerror(errno) <<
                                                                        ")" << std::endl;

            server_socket_close();
            return IMC_FAIL;
        } else {
            std::cout << TAG_INFO << "Server socket listening(fd => " << server_socket_fd << ")" << std::endl;
        }

        server_socket_started = true;
        return IMC_SUCCESS;
    } else {
        std::cerr << TAG_ERROR << "Cannot start server socket, server socket already started(fd => "
                                                                                << server_socket_fd << ")" << std::endl;
        return IMC_FAIL;
    }
}

int SocketServer::stop() {
    if(server_socket_started) {
        server_socket_close();
        return IMC_SUCCESS;
    } else {
        std::cerr << TAG_ERROR << "Cannot stop server socket, server socket not started" << std::endl;
        return IMC_FAIL;
    }
}

int SocketServer::accept_connection() {
    if(server_socket_started) {
        // Accept
        struct sockaddr_in client_socket_address;
        memset(&client_socket_address, 0, sizeof(client_socket_address));

        int client_socket_address_length = sizeof(client_socket_address);
        int client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client_socket_address,
                                      (socklen_t *) &client_socket_address_length);

        if (client_socket_fd < 0) {
            std::cerr << TAG_ERROR << "Error accepting client socket(errno =>" << strerror(errno) << ")" << std::endl;
            return IMC_FAIL;
        } else {
            std::cout << TAG_INFO << "Accepted client socket(fd => " << client_socket_fd << ")" << std::endl;
        }

        // Handle client socket
        handle_client_socket(client_socket_fd);

        // Close
        int close_result = close(client_socket_fd);

        if (close_result < 0) {
            std::cerr << TAG_ERROR << "Error closing client socket(fd => " << client_socket_fd << ")" << std::endl;
            return IMC_FAIL;
        } else {
            std::cout << TAG_INFO << "Client socket closed(fd => " << client_socket_fd << ")" << std::endl;
        }

        return IMC_SUCCESS;
    } else {
        std::cerr << TAG_ERROR << "Cannot accept connections, server socket not started" << std::endl;
        return IMC_FAIL;
    }
}

int SocketServer::socket_receive(int socket_fd, char *receive_buffer, int receive_buffer_length) {
    bzero(receive_buffer, receive_buffer_length);

    int receive_result = recv(socket_fd, receive_buffer, receive_buffer_length, 0);

    if(receive_result < 0) {
        std::cerr << TAG_ERROR << "Failed to receive from socket(fd => " << socket_fd <<
                                                                ", errno => " << strerror(errno) <<
                                                                 ")" << std::endl;
        return IMC_FAIL;
    } else {
        if(cout_socket_receive) {
            std::cout << TAG_DEBUG << "Received from socket(fd => " << socket_fd << std::endl << TAG_NONE <<
            "                     receive_buffer => " << receive_buffer << ")" << std::endl;
        }

        return IMC_SUCCESS;
    }
}

int SocketServer::socket_send(int socket_fd, char *send_buffer, int send_buffer_length) {
    int send_result = send(socket_fd, send_buffer, send_buffer_length, 0);

    if(send_result < 0) {
        std::cerr << TAG_ERROR << "Failed to send from socket(fd => " << socket_fd <<
                                                             ", errno => " << strerror(errno) <<
                                                              ")" << std::endl;
        return IMC_FAIL;
    } else {
        if(cout_socket_send) {
            std::cout << TAG_DEBUG << "Sent from socket(fd => " << socket_fd << std::endl << TAG_NONE <<
            "                 send_buffer => " << send_buffer << ")" << std::endl;
        }

        return IMC_SUCCESS;
    }
}