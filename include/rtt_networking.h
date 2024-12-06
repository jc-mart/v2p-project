#ifndef RTT_NETWORKING_H
#define RTT_NETWORKING_H

#include <netdb.h>
#include <rtt_helper.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BACKLOG 10
#define MAXDATASIZE 8
#define PAYLOAD "PING"
#define FIN "FIN"

int create_and_bind(struct addrinfo *server_info, int *sock_fd, int *options);
int measure_rtt(int sock_fd, int iterations, double *results);
void *get_in_addr(struct sockaddr *sa);

#endif
