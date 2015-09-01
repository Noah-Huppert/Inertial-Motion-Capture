#include <iostream>
#include <thread>
#include <string>
#include <signal.h>

#include <libsocket/inetserverdgram.hpp>
#include <libsocket/exception.hpp>
#include <signal.h>

int socket_receive_buffer_size = 256;

std::string socket_host = "localhost";
std::string socket_port = "1234";

void kill_server(int signal) {
    std::cout << "CTRL+C received" << std::endl;
    exit(0);
}

void run_server() {
    try {
        libsocket::inet_dgram_server socket_server(socket_host, socket_port, LIBSOCKET_BOTH);

        std::cout << "Running socket on " << socket_host << ":" << socket_port << std::endl;

        /*
         rcvfrom	(	void * 	buf,
                        size_t 	len,
                        string & 	srchost,
                        string & 	srcport,
                        int 	rcvfrom_flags = 0,
                        bool 	numeric = false
                    )
         */

        char receive_buffer[socket_receive_buffer_size];

        while (true) {
            socket_server.rcvfrom(receive_buffer, socket_receive_buffer_size, socket_host, socket_port);

            std::cout << "Socket recieved: " << receive_buffer << std::endl;

            break;
        }
    } catch (const libsocket::socket_exception &exception) {
        std::cout << "Socket exception: " << exception.mesg << std::endl;
        kill_server(-1);
    }
}

int main() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = kill_server;
    sigIntHandler.sa_flags = 0;
    sigemptyset(&sigIntHandler.sa_mask);

    sigaction(SIGINT, &sigIntHandler, NULL);

    std::thread server_thread(run_server);

    server_thread.join();

    return 0;
}

/*
void server() {
    using std::string;

    string host = "localhost";
    string port = "1234";

    string answer("Hello back from the server!");
    string from;
    string fromport;
    string buf;

    buf.resize(32);

    try {
        libsocket::inet_dgram_server srv(host, port, LIBSOCKET_BOTH);

        printf("Running socket on %s:%s", host.c_str(), port.c_str());

        while (true) {
            srv.rcvfrom(buf, from, fromport);

            std::cout << "Datagram from " << from << ":" << fromport << " " << buf << std::endl;

            srv.sndto(answer, from, fromport);
        }

        srv.destroy();
    } catch (const libsocket::socket_exception &exc) {
        std::cerr << exc.mesg;
    }
}

int main() {
    std::thread server_thread(server);

    printf("RUNNING");

    while (true) {
        std::string command;
        std::getline (std::cin, command);

        printf("COMMAND => %s", command.c_str());

        if(command == "q" || command == "quit") {
            running = false;
            break;
        }
    }

    server_thread.join();
}
*/