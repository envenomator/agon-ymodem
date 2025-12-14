#define _POSIX_C_SOURCE 200809L

#include "millis.h"
#include <time.h>
#include <stdint.h>

uint64_t millis(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

