/**
 * @file rtt_helper.c
 * @brief This file contains helper functions for calculating round-trip time
 * (RTT).
 */
#include "rtt_helper.h"

/**
 * @brief Calculates the elapsed time between two time points.
 *
 * The function calculates the elapsed time in milliseconds between the start
 * and end time points.
 *
 * @param start Pointer to the timeval structure representing the start time.
 * @param end Pointer to the timeval structure representing the end time.
 * @return The elapsed time in milliseconds.
 */
double time_elapsed(const struct timeval *start, const struct timeval *end) {
    return ((end->tv_sec - start->tv_sec) * 1000.0) +
           ((end->tv_usec - start->tv_usec) / 1000.0);
}