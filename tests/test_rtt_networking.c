#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/wait.h>

#include "rtt_networking.h"
#include "unity.h"

#define TESTPORT "2424"
#define TESTPAYLOAD "TEST"

int yes = 1;
int test_sock, client_fd;
struct addrinfo test_hints, *test_info, client_hints, *client_info;
struct sockaddr_in test_sockname;
socklen_t test_socklen = sizeof test_sockname;

void setUp(void) {
    test_sock = -1;
    client_fd = -1;
}

void tearDown(void) {
    if (test_sock != -1) close(test_sock);
    if (client_fd != -1) close(client_fd);
}

void setup_server(int *socket, struct addrinfo hints, struct addrinfo *info,
                  int *options) {
    // Ensure structures are zeroed out
    memset(&hints, 0, sizeof hints);
    memset(&info, 0, sizeof info);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, TESTPORT, &hints, &info);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Failed to get address info");

    status = create_and_bind(info, socket, options);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Create and bind funciton failed");
}

void setup_client(int *client_socket, struct addrinfo hints,
                  struct addrinfo *info) {
    struct addrinfo *ptr;

    memset(&hints, 0, sizeof hints);
    memset(&info, 0, sizeof info);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", TESTPORT, &hints, &info) != 0) {
        perror("getaddrinfo");
        TEST_ASSERT_FALSE_MESSAGE(-1, "[CLIENT] getaddrinfo failed");
    }

    for (ptr = info; ptr != NULL; ptr = ptr->ai_next) {
        if ((*client_socket = socket(ptr->ai_family, ptr->ai_socktype,
                                     ptr->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(*client_socket, ptr->ai_addr, ptr->ai_addrlen) == -1) {
            close(*client_socket);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (ptr == NULL)
        TEST_ASSERT_FALSE_MESSAGE(-1, "[CLIENT] failed to connect");
    freeaddrinfo(info);
}

void test_create_and_bind(void) {
    setup_server(&test_sock, test_hints, test_info, &yes);

    int status = getsockname(test_sock, (struct sockaddr *)&test_sockname,
                             &test_socklen);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Failed to get socket info");

    int actual = ntohs(test_sockname.sin_port);
    TEST_ASSERT_EQUAL_MESSAGE(2424, actual, "Incorrect socket");
}

// Basics will ensure correctness of `create_and_bind()`
void basic_server(void) {
    int new_fd;
    struct sockaddr_storage incoming_addr;
    socklen_t addr_size;
    char buf[MAXDATASIZE];
    int num_bytes;

    setup_server(&test_sock, test_hints, test_info, &yes);
    addr_size = sizeof incoming_addr;
    new_fd = accept(test_sock, (struct sockaddr *)&incoming_addr, &addr_size);

    if (new_fd == -1) {
        perror("accept");
        TEST_ASSERT_FALSE_MESSAGE(new_fd,
                                  "[SERVER] Failed to accept connection");
    }

    num_bytes = recv(new_fd, buf, MAXDATASIZE - 1, 0);
    if (num_bytes == -1) {
        perror("recv");
        TEST_ASSERT_FALSE_MESSAGE(new_fd, "[SERVER] Failed to recieve message");
    }

    buf[num_bytes] = '\0';
    TEST_ASSERT_EQUAL_STRING(TESTPAYLOAD, buf);

    close(new_fd);
    close(test_sock);
}

// Basics will ensure the correctness of `create_and_bind()`
void basic_client(void) {
    int num_bytes;
    char buf[MAXDATASIZE];

    setup_client(&client_fd, client_hints, client_info);

    strncpy(buf, TESTPAYLOAD, MAXDATASIZE - 1);
    num_bytes = send(client_fd, buf, strlen(buf), 0);
    if (num_bytes == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes, "[CLIENT] failed to send message");
    }

    close(client_fd);
}

// Protocols will ensure the correctness of `measure_rtt()`
void protocol_server(void) {
    int new_fd;
    struct sockaddr_storage incoming_addr;
    socklen_t addr_size;
    char buf[MAXDATASIZE];
}

// Protocols will ensure the correctness of `measure_rtt()`
void protocol_client(void) {}

void test_socket_comms(void) { fork() ? basic_client() : basic_server(); }

void test_measure_rtt(void) {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_bind);
    RUN_TEST(test_socket_comms);
    return UNITY_END();
}
