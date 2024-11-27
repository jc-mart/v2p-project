#ifndef RTT_HELPER_H
#define RTT_HELPER_H

#include <sys/time.h>

double time_elapsed(const struct timeval *start, const struct timeval *end);

#endif
