#include <unistd.h>

#include "rtt_helper.h"
#include "time.h"
#include "unity.h"
#include "wait.h"

static struct timeval start = {0};
static struct timeval end = {0};

void setUp(void) {
    gettimeofday(&start, NULL);
    sleep(5);

    gettimeofday(&end, NULL);
}

void tearDown(void) {
    // Cleans up code, if it's needed
}

void test_time_elapsed() {
    double expected = ((end.tv_sec - start.tv_sec) * 1000.0) +
                      ((end.tv_usec - start.tv_usec) / 1000.0);
    double actual = time_elapsed(&start, &end);

    TEST_ASSERT_EQUAL(expected, actual);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_time_elapsed);
    return UNITY_END();
}
