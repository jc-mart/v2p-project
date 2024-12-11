#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <sys/wait.h>

#include "rtt_networking.h"
#include "unity.h"

#define TESTPORT "2424"
#define TESTPAYLOAD "TEST"
#define ACKNOWLEDGE "ACK"
#define TIMINGSIZE 5

int yes = 1;
int host_fd, client_fd, host_new_fd;
struct addrinfo test_hints, *test_info, client_hints, *client_info;
struct sockaddr_in test_sockname;
socklen_t test_socklen = sizeof test_sockname;

void setUp(void) {
    host_fd = -1;
    host_new_fd = -1;
    client_fd = -1;
}

void tearDown(void) {
    if (host_fd != -1) {
        close(host_fd);
    }
    if (host_new_fd != -1) {
        close(host_new_fd);
    }
    if (client_fd != -1) {
        close(client_fd);
    }
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
    setup_server(&host_fd, test_hints, test_info, &yes);

    int status =
        getsockname(host_fd, (struct sockaddr *)&test_sockname, &test_socklen);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Failed to get socket info");

    int actual = ntohs(test_sockname.sin_port);
    TEST_ASSERT_EQUAL_MESSAGE(2424, actual, "Incorrect socket");
}

// Basics will ensure correctness of `create_and_bind()`
void basic_server(void) {
    int num_bytes;
    struct sockaddr_storage incoming_addr;
    socklen_t addr_size;
    char buf[MAXDATASIZE];

    setup_server(&host_fd, test_hints, test_info, &yes);
    addr_size = sizeof incoming_addr;
    host_new_fd =
        accept(host_fd, (struct sockaddr *)&incoming_addr, &addr_size);

    if (host_new_fd == -1) {
        perror("accept");
        TEST_ASSERT_FALSE_MESSAGE(host_new_fd,
                                  "[SERVER] Failed to accept connection");
    }

    num_bytes = recv(host_new_fd, buf, MAXDATASIZE - 1, 0);
    if (num_bytes == -1) {
        perror("recv");
        TEST_ASSERT_FALSE_MESSAGE(host_new_fd,
                                  "[SERVER] Failed to recieve message");
    }

    buf[num_bytes] = '\0';
    TEST_ASSERT_EQUAL_STRING(TESTPAYLOAD, buf);

    close(host_new_fd);
    close(host_fd);
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
    int num_bytes;
    struct sockaddr_storage incoming_addr;
    socklen_t addr_size;
    double timings[TIMINGSIZE];
    char buf[MAXDATASIZE];

    printf("server setting up\n");
    setup_server(&host_fd, test_hints, test_info, &yes);
    addr_size = sizeof incoming_addr;

    printf("server is accepting new connection\n");
    host_new_fd =
        accept(host_fd, (struct sockaddr *)&incoming_addr, &addr_size);

    if (host_new_fd == -1) {
        perror("accept");
        TEST_ASSERT_FALSE_MESSAGE(host_new_fd,
                                  "[SERVER] Failed to accept connection");
    }

    printf("server sending ack to client\n");
    // acknowledge the client
    strncpy(buf, ACKNOWLEDGE, MAXDATASIZE - 1);
    num_bytes = send(host_new_fd, buf, strlen(buf), 0);
    if (num_bytes == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes, "[SERVER] failed to send ping");
    }

    sleep(1);  // mimick some delay
    printf("server receiving ack from client\n");
    // recieve acknowledge from client
    num_bytes = recv(host_new_fd, buf, MAXDATASIZE - 1, 0);
    if (num_bytes == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes, "[SERVER] failed to recieve ping");
    }

    buf[num_bytes] = '\0';
    TEST_ASSERT_EQUAL_STRING(ACKNOWLEDGE, buf);

    /**
     * @todo introduce measure_rtt protocol; have
     * client mimick a ping. global client_fd for server
     * and client to close sockets in case of either failure
     * or failed test condition
     */
    printf("server beginning protocol\n");
    measure_rtt(host_new_fd, TIMINGSIZE, timings);

    // Protocol complete, send finish message
    strncpy(buf, FIN, MAXDATASIZE - 1);
    printf("server sending finish to client\n");
    if ((num_bytes = send(host_new_fd, FIN, strlen(buf), 0)) == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes, "Failed to send finish to client");
    }

    // Make sure that client did recieve finish message
    printf("server receiving confirmation from client\n");
    if ((num_bytes = recv(host_new_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes,
                                  "Failed to recieve finish from client");
    }

    close(host_new_fd);
    close(host_fd);
}

// Protocols will ensure the correctness of `measure_rtt()`
void mimick_protocol_client(void) {
    int num_bytes, result;
    char buf[MAXDATASIZE];

    printf("client setting up\n");
    setup_client(&client_fd, client_hints, client_info);
    printf("client receiving host ack\n");
    // recieve host acknowledgement
    if ((num_bytes = recv(client_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        TEST_ASSERT_FALSE_MESSAGE(
            num_bytes, "Failed to recieve acknowledgement from server");
    }

    buf[num_bytes] = '\0';
    TEST_ASSERT_EQUAL_STRING(ACKNOWLEDGE, buf);
    printf("client sending ack to host\n");
    // send acknowledgemet to host
    strncpy(buf, ACKNOWLEDGE, MAXDATASIZE - 1);
    if ((num_bytes = send(client_fd, buf, strlen(buf), 0)) == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes,
                                  "Failed to send acknowledgement to server");
    }
    // recieve ping for x iterations
    // return ping for x iterations
    printf("client entering for loop\n");
    while (1) {
        if ((num_bytes = recv(client_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
            perror("recv");
            TEST_ASSERT_FALSE_MESSAGE(num_bytes,
                                      "Failed to recieve server ping");
        }

        buf[num_bytes] = '\0';
        // Leave loop if finish message was recieved
        if (!strncmp(FIN, buf, strlen(FIN))) break;
        TEST_ASSERT_EQUAL_STRING(PAYLOAD, buf);

        // Artificial delay
        sleep(1);

        strncpy(buf, PAYLOAD, MAXDATASIZE - 1);
        if ((num_bytes = send(client_fd, buf, strlen(buf), 0)) == -1) {
            perror("send");
            TEST_ASSERT_FALSE_MESSAGE(num_bytes,
                                      "Failed to send ping to server");
        }
    }

    printf("client exited for loop, finishing interaction\n");
    strncpy(buf, FIN, MAXDATASIZE - 1);
    if ((num_bytes = send(client_fd, buf, strlen(buf), 0)) == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes,
                                  "Failed to send finish message to host");
    }

    close(client_fd);
}

void protocol_client(void) {
    int num_bytes;
    char buf[MAXDATASIZE];

    printf("client setting up\n");
    setup_client(&client_fd, client_hints, client_info);
    printf("client receiving host ack\n");
    // recieve host acknowledgement
    if ((num_bytes = recv(client_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        TEST_ASSERT_FALSE_MESSAGE(
            num_bytes, "Failed to recieve acknowledgement from server");
    }

    buf[num_bytes] = '\0';
    TEST_ASSERT_EQUAL_STRING(ACKNOWLEDGE, buf);
    printf("client sending ack to host\n");
    // send acknowledgemet to host
    strncpy(buf, ACKNOWLEDGE, MAXDATASIZE - 1);
    if ((num_bytes = send(client_fd, buf, strlen(buf), 0)) == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes,
                                  "Failed to send acknowledgement to server");
    }

    relay_ping(client_fd);

    printf("client exited for loop, finishing interaction\n");
    strncpy(buf, FIN, MAXDATASIZE - 1);
    if ((num_bytes = send(client_fd, buf, strlen(buf), 0)) == -1) {
        perror("send");
        TEST_ASSERT_FALSE_MESSAGE(num_bytes,
                                  "Failed to send finish message to host");
    }

    close(client_fd);
}

void test_socket_comms(void) {
    if (fork()) {
        basic_client();
        wait(NULL);
    } else {
        basic_server();
        exit(0);  // Terminate child process to prevent multiple tests
    }
}

void test_measure_rtt(void) {
    printf("beginning measure rtt test\n");

    if (fork()) {
        protocol_server();
        wait(NULL);
    } else {
        mimick_protocol_client();
        exit(0);
    }
}

void test_relay_ping(void) {
    printf("beginning relay ping test\n");

    if (fork()) {
        protocol_server();
        wait(NULL);
    } else {
        protocol_client();
        exit(0);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_bind);
    sleep(1);
    RUN_TEST(test_socket_comms);
    sleep(1);
    RUN_TEST(test_measure_rtt);
    sleep(1);
    RUN_TEST(test_relay_ping);

    return UNITY_END();
}
