#define _GNU_SOURCE

#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <wait.h>

#include "file_handler.h"
#include "unity.h"

#define TESTDIR "logs"

struct stat info = {0};
double test_arr[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
int test_arr_size = 10;
struct timeval timestamp;

void setUp(void) {
    if (stat(TESTDIR, &info) == 0)  // Directory is present
        rmdir(TESTDIR);

    memset(&info, 0, sizeof info);

    gettimeofday(&timestamp, NULL);
}

void tearDown(void) {
    // if (stat(TESTDIR, &info) == 0) rmdir(TESTDIR);
}

void test_create_dir(void) {
    int actual = create_dir(TESTDIR);
    TEST_ASSERT_EQUAL(0, actual);
}

void test_log_rtt(void) {
    int actual = log_rtt(test_arr, test_arr_size, &timestamp.tv_sec);
    // TEST_ASSERT_EQUAL(0, actual);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_dir);
    RUN_TEST(test_log_rtt);
    return UNITY_END();
}
