#include "rtt_helper.h"

double time_elapsed(const struct timeval *start, const struct timeval *end) {
    return ((end->tv_sec - start->tv_sec) * 1000.0) +
           ((end->tv_usec - start->tv_usec) / 1000.0);
}