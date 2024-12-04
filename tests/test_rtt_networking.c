#define _GNU_SOURCE

#include <sys/wait.h>

#include "rtt_networking.h"
#include "unity.h"

#define TESTPORT "2424"
#define ESTPORT "2425"
#define LOCALHOST "127.0.0.1"
#define PAYLOAD "TEST PAYLOAD"
#define MAXSIZE 32

int yes = 1;
int test_sock, est_sock;
struct addrinfo *test_hints, *test_info, *est_hints, *est_info;
struct sockaddr_storage incoming_address;
struct sockaddr_in test_sockname;
socklen_t test_socklen = sizeof test_sockname;

void setUp(void) {
    test_sock = -1;
    est_sock = -1;
}

void tearDown(void) {
    if (test_sock != -1) close(test_sock);
    if (est_sock != -1) close(est_sock);
}

void setup_socket(int *socket, struct addrinfo *hints, struct addrinfo *info,
                  int *options) {
    // Ensure structures are zeroed out
    memset(&hints, 0, sizeof hints);
    memset(&info, 0, sizeof info);

    int status = getaddrinfo(NULL, TESTPORT, hints, &info);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Failed to get address info");

    status = create_and_bind(info, socket, options);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Create and bind funciton failed");
}

void test_create_and_bind(void) {
    setup_socket(&test_sock, test_hints, test_info, &yes);

    int status = getsockname(test_sock, (struct sockaddr *)&test_sockname,
                             &test_socklen);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Failed to get socket info");

    int actual = ntohs(test_sockname.sin_port);
    TEST_ASSERT_EQUAL_MESSAGE(2424, actual, "Incorrect socket");
}

void test_socket_connection(void) {
    pid_t pid = fork();

    if (pid == 0) {
        // Listener setup
        int recieved = 0;
        int new_fd, num_bytes;
        char buf[MAXSIZE];
        socklen_t incoming_sin_size = {0};

        setup_socket(&test_sock, test_hints, test_info, &yes);

        while (!recieved) {
            new_fd = accept(test_sock, (struct sockaddr *)&incoming_address,
                            &incoming_sin_size);
            if (new_fd == -1) {
                perror("Accept failed");
                continue;
            }

            num_bytes = recv(new_fd, buf, MAXDATASIZE - 1, 0);
            TEST_ASSERT_NOT_EQUAL_MESSAGE(-1, num_bytes, "Recieve failed");

            buf[num_bytes] = '\0';
            TEST_ASSERT_EQUAL_STRING(PAYLOAD, buf);
        }

    } else {
        // Standby for listener setup
        sleep(1);
        // Sender setup
        int est_status;
        struct addrinfo *ptr;

        memset(&est_hints, 0, sizeof est_hints);
        est_hints->ai_family = AF_UNSPEC;
        est_hints->ai_socktype = SOCK_STREAM;

        est_status = getaddrinfo(LOCALHOST, TESTPORT, &est_hints, &est_info);
        TEST_ASSERT_EQUAL_MESSAGE(0, est_status, "Failed to get address info");

        // Connect to test socket
        for (ptr = est_info; ptr != NULL; ptr = ptr->ai_next) {
            if ((est_sock = socket(ptr->ai_family, ptr->ai_socktype,
                                   ptr->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }

            if (connect(est_sock, ptr->ai_addr, ptr->ai_addrlen) == -1) {
                close(est_sock);
                perror("client: connect");
                continue;
            }

            break;
        }
        TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, ptr, "Client failed to connect");

        freeaddrinfo(est_info);

        est_status = send(est_sock, PAYLOAD, sizeof PAYLOAD, 0);
        TEST_ASSERT_EQUAL_MESSAGE(0, est_status, "Failed to send payload");

        close(est_sock);
    }

    // Set up test socket

    // Set up another socket to send a message

    // Confirm delivery

    // Close sockets
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_bind);
    return UNITY_END();
}
