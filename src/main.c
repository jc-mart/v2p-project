/**
 * @brief Calculates the elapsed time between two time points.
 *
 * The function calculates the elapsed time in milliseconds between the start
 * and end time points.
 *
 * @param start Pointer to the timeval structure representing the start time.
 * @param end Pointer to the timeval structure representing the end time.
 * @return The elapsed time in milliseconds.
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file_handler.h"
#include "process_helper.h"
#include "rtt_helper.h"
#include "rtt_networking.h"

#define PORT "2424"
#define LOGPATH "../logs/"

/**
 * @brief Main entry point for the server application.
 *
 * The function initializes the server, creates and binds a socket, listens for
 * incoming connections, and measures the round-trip time (RTT) for messages.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return 0 on success, 1 on failure.
 */
int main(const int argc, const char *argv[]) {
    int sock_fd, status, rv, iterations;
    int yes = 1;
    sock_fd = 0;
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size;
    struct addrinfo hints, *server_info;
    struct sockaddr_storage incoming_addr;
    struct sigaction sa;

    if (argc != 3) {
        fprintf(stderr, "usage: server_rtt <hostname> <iterations>");
        return 1;
    }

    iterations = strtol(argv[2], (char **)NULL, 10);

    // TODO Create log directory if not created already
    if ((status = create_dir(LOGPATH)) != 0) {
        fprintf(stderr, "create_dir failed: %d\n", status);
        return 1;
    }

    // Clear memory space for address information
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Server creates and binds to a socket instance
    if ((status = create_and_bind(server_info, &sock_fd, &yes)) != 0) {
        fprintf(stderr, "create_and_bind failed: %d\n", status);
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections\n");

    // Passively accept new connections and perform protocol to retrieve RTT
    while (1)  // Find a way for keyboard interrupt handling
    {
        int new_fd;

        sin_size = sizeof incoming_addr;
        new_fd = accept(sock_fd, (struct sockaddr *)&incoming_addr, &sin_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(incoming_addr.ss_family,
                  get_in_addr((struct sockaddr *)&incoming_addr), s, sizeof s);
        printf("server: connected to %s\n", s);

        if (!fork()) {  // Forks subsequent interactions to a child process
            // TODO PUT MEASURE RTT FUNCTION HERE WITH ERROR HANDLING
            double results[10];
            struct timeval start;
            gettimeofday(&start, NULL);
            measure_rtt(sock_fd, iterations, results);
            log_rtt(results, 10, &start.tv_sec);
            close(sock_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}
