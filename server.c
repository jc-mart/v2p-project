#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define PORT "2424"
#define BACKLOG 10
#define PAYLOAD "PING\0  "
#define MAXDATASIZE 8

// Prototypes
// TODO: Split functions into files based on functionality.
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *s);
int create_and_bind(struct addrinfo *server_info, int *sock_fd, int *options);
int measure_rtt(int sock_fd, int iterations);
int log_rtt();

int main(int argc, char *argv[])
{
    int sock_fd, new_fd, status, rv, iterations;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size;
    struct addrinfo hints, *server_info;
    struct sockaddr_storage incoming_addr;
    struct sigaction sa;

    if (argc != 3)
    {
        fprintf(stderr, "usage: server_rtt <hostname> <iterations>");
        return 1;
    }

    // Clear memory space for address information
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Server creates and binds to a socket instance
    if ((status = create_and_bind(server_info, sock_fd, yes)) != 0)
    {
        fprintf(stderr, "create_and_bind failed: %d\n", status);
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections\n");

    // Passively accept new connections and perform protocol to retrieve RTT
    while (1) // Find a way for keyboard interrupt handling
    {
        sin_size = sizeof incoming_addr;
        new_fd = accept(sock_fd, (struct sockaddr *)&incoming_addr, &sin_size);

        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(incoming_addr.ss_family,
                  get_in_addr((struct sockaddr *)&incoming_addr),
                  s, sizeof s);
        printf("server: connected to %s\n", s);

        
        if (!fork()) { // Forks subsequent interactions to a child process
            // TODO PUT MEASURE RTT FUNCTION HERE WITH ERROR HANDLING

            close(sock_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}

void sigchld_handler(int s)
{
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// Adjusts between IPv4 and v6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return found struct information to `p`
int create_and_bind(struct addrinfo *server_info, int *sock_fd, int *options)
{
    struct addrinfo *potential;

    for (potential = server_info; potential != NULL; potential = potential->ai_next)
    {
        if ((*sock_fd = socket(potential->ai_family, potential->ai_socktype,
                               potential->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue; // Goes onto next element of linked list
        }

        if (setsockopt(*sock_fd, SOL_SOCKET, SO_REUSEADDR, options, sizeof(int)) == -1)
        {
            perror("setsockopt");
            close(*sock_fd);
            return -1; // Something fatal, Beej's book exits upon failure
        }

        if (bind(*sock_fd, potential->ai_addr, potential->ai_addrlen) == -1)
        {
            close(sock_fd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(server_info); // Binded to server socket, no need for this

    if (potential == NULL)
    {
        fprintf(stderr, "server: failed to bind socket\n");
        return -2;
    }

    if (listen(*sock_fd, BACKLOG) == -1)
    {
        perror("listen");
        return -3;
    }

    return 0; // Successfully binded to a socket
}

// can create its own file to log the rtt's
int measure_rtt(int sock_fd, int iterations)
{
    struct timeval start, end;
    int numbytes;
    double timings[10];
    char buf[MAXDATASIZE];

    for (int i = 0; i < iterations; i++) {
        
        gettimeofday(&start, NULL);

        if (send(sock_fd, PAYLOAD, sizeof PAYLOAD, 0) == -1)
        {
            perror("send");
            return -1;
        }

        if ((numbytes = recv(sock_fd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            return -2;
        }

        gettimeofday(&end, NULL);
        buf[numbytes] = '\0';

        // Check if recieved messages was expected, valid pass else abort
        if (buf == PAYLOAD);
    }
    return 0;
}

int log_rtt() {
    return 0;
}