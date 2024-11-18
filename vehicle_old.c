#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVICE_PORT 2424
#define BACKLOG 10

int main(int argc, char *argv[]) {

    struct sockaddr_storage incoming_address;
    socklen_t address_size;
    struct addrinfo local_hints, *local_response;
    int local_fd, incoming_fd, status;

    // Error handling
    if (argc != 2) {
        fprintf(stderr, "usage: <program_name> <destination_ip>");
        return 1;
    }

    memset(&local_hints, 0, sizeof local_hints);
    local_hints.ai_family = AF_UNSPEC;
    local_hints.ai_socktype = SOCK_STREAM;
    local_hints.ai_flags = AI_PASSIVE;

    if (status = (getaddrinfo(NULL, SERVICE_PORT, &local_hints, &local_response)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    // if (status = (getaddrinfo(argv[1], SERVICE_PORT, &incoming_hints, &incoming_response)) != 0) {
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    //     return 3;
    // }

    // Create a socket, bind to it, and listen
    local_fd = socket(local_response->ai_family, local_response->ai_socktype, local_response->ai_protocol);
    bind(local_fd, local_response->ai_addr, local_response->ai_addrlen);
    listen(local_fd, BACKLOG);

    // Accept incoming connection
    address_size = sizeof incoming_address;
    incoming_fd = accept(local_fd, (struct sockaddr *)&incoming_address, &address_size);

}
