#define _GNU_SOURCE

#include "rtt_networking.h"
#include "unity.h"

#define TESTPORT "2424"

int yes = 1;
int test_sock, est_sock;
struct addrinfo *test_hints, *test_info;
struct sockaddr_storage test_addr;
struct sockaddr_in test_sockname;
socklen_t test_socklen = sizeof test_sockname;

void setUp(void) { test_sock = -1; }

void tearDown(void) {
    if (test_sock != -1) close(test_sock);
}

void setup_test_socket(void) {
    memset(&test_hints, 0, sizeof test_hints);
    memset(&test_info, 0, sizeof test_info);
    int status = getaddrinfo(NULL, TESTPORT, test_hints, &test_info);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Failed to get address info");

    status = create_and_bind(test_info, &test_sock, &yes);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Create and bind function failed");
}

void test_create_and_bind(void) {
    setup_test_socket();
    int status = getsockname(test_sock, (struct sockaddr *)&test_sockname,
                             &test_socklen);
    TEST_ASSERT_EQUAL_MESSAGE(0, status, "Failed to get socket info");

    int actual = ntohs(test_sockname.sin_port);
    TEST_ASSERT_EQUAL_MESSAGE(2424, actual, "Incorrect socket");
}

void test_socket_connection(void) { setup_test_socket(); }

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_bind);
    return UNITY_END();
}
