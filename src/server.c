#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PORT "2424"
#define BACKLOG 10
#define PAYLOAD "PING"
#define MAXDATASIZE 8
#define FILENAMESIZE 23

// Prototypes
// TODO: Split functions into files based on functionality.
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *s);
int create_and_bind(struct addrinfo *server_info, int *sock_fd, int *options);
int measure_rtt(int sock_fd, int iterations, double *results[]);
double time_elapsed(struct timeval *start, struct timeval *end);
int log_rtt(double *data[], int data_size, time_t *time);

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

void sigchld_handler(int s) {
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// Adjusts between IPv4 and v6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

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
            close(sock_fd);
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

// can create its own file to log the rtt's
int measure_rtt(int sock_fd, int iterations, double *results[]) {
    struct timeval start, end;
    int numbytes;
    double elapsed;
    double timings[iterations];
    char buf[MAXDATASIZE];

    for (int i = 0; i < iterations; i++) {
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
        printf("pass %d: %ld\n", i, elapsed);
        timings[i] = elapsed;
    }
    return 0;
}

double time_elapsed(struct timeval *start, struct timeval *end) {
    return ((end->tv_sec - start->tv_sec) * 1000.0) +
           ((end->tv_usec - start->tv_usec) / 1000.0);
}

// TODO make sure in the main function to ensure that a log directory is present
int log_rtt(double *data[], int data_size, time_t *time) {
    // Convert epoch time to mmdd_hh:mm:ss
    char buf[FILENAMESIZE];
    strftime(buf, FILENAMESIZE, "./logs/rtt_%m%d_%H%M%S", localtime(&time));

    FILE *file = fopen(buf, "w");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    fprintf(file,
            "rtt for %s\n \
            pass no., time (ms)\n",
            buf + 4);
    for (int i = 0; i < data_size; i++) {
        fprintf(file, "%d, %f\n", i, data[i]);
    }
    fclose(file);

    return 0;
}