#pragma once

#ifndef _MSC_VER
#include <unistd.h>
#else
void usleep(unsigned int usec);
#define CLOCK_MONOTONIC 1
int clock_gettime(int X, struct timespec* tv);

#endif
