/**
 * @file rtt-networking.c
 * @brief This file contains networking code that creates sockets and runs a
 *  protocol between two devices.
 */
#include "rtt_networking.h"

/**
 * @brief Creates a socket to listen for incoming messages.
 *
 * The function creates a socket, binds to it, and listens for incoming
 * messages.
 *
 * @param server_info Linked list of potential sockets the function can connect
 * to
 * @param sock_fd Integer that holds the socket's file descriptor.
 * @param options Integer that holds the socket's configuration settings.
 */
// Return found struct information to `p`
int create_and_bind(struct addrinfo *server_info, int *sock_fd, int *options) {
    struct addrinfo *potential;

    for (potential = server_info; potential != NULL;
         potential = potential->ai_next) {
        if ((*sock_fd = socket(potential->ai_family, potential->ai_socktype,
                               potential->ai_protocol)) == -1) {
            perror("server: socket");
            continue;  // Goes onto next element of linked list
        }

        if (setsockopt(*sock_fd, SOL_SOCKET, SO_REUSEADDR, options,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            close(*sock_fd);
            return -1;  // Something fatal, Beej's book exits upon failure
        }

        if (bind(*sock_fd, potential->ai_addr, potential->ai_addrlen) == -1) {
            close(*sock_fd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(server_info);  // Binded to server socket, no need for this

    if (potential == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return -2;
    }

    if (listen(*sock_fd, BACKLOG) == -1) {
        perror("listen");
        return -3;
    }

    return 0;  // Successfully binded to a socket
}

// Adjusts between IPv4 and v6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// can create its own file to log the rtt's
int measure_rtt(int sock_fd, int iterations, double *results[]) {
    struct timeval start, end;
    double elapsed;
    double timings[iterations];
    char buf[MAXDATASIZE];

    for (int i = 0; i < iterations; i++) {
        int numbytes;
        gettimeofday(&start, NULL);

        if (send(sock_fd, PAYLOAD, sizeof PAYLOAD, 0) == -1) {
            perror("send");
            return -1;
        }

        if ((numbytes = recv(sock_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
            perror("recv");
            return -2;
        }

        gettimeofday(&end, NULL);
        buf[numbytes] = '\0';

        // Check if recieved messages was expected, valid pass else abort
        if (strcmp(buf, PAYLOAD) != 0) {
            fprintf(stderr,
                    "server: failed to recieve correct payload and retrying\n");
            i = (i > 0) ? (i - 1) : 0;
            continue;
        }

        // Put data in array, for logging after in milliseconds
        elapsed = time_elapsed(&start, &end);
        printf("pass %d: %f\n", i, elapsed);
        timings[i] = elapsed;
    }
    return 0;
}
