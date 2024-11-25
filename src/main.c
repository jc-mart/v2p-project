#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "file_handling/file_handler.h"
#include "networking/rtt_networking.h"
#include "rtt_functions/rtt_helper.h"
#include "process/process_helper.h"

#define PORT "2424"
#define LOGPATH "../logs/"

int main(int argc, char *argv[]) {
    int sock_fd, new_fd, status, rv, iterations;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size;
    struct addrinfo hints, *server_info;
    struct sockaddr_storage incoming_addr;
    struct sigaction sa;

    if (argc != 3) {
        fprintf(stderr, "usage: server_rtt <hostname> <iterations>");
        return 1;
    }
    // TODO Create log directory if not created already
    if (status = create_dir(LOGPATH) != 0) {
        fprintf(stderr, "create_dir failed: %s\n", status);
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
    if ((status = create_and_bind(server_info, sock_fd, yes)) != 0) {
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

            close(sock_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}
