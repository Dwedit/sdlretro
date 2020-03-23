#pragma once

void usleep(unsigned int usec);
#define CLOCK_MONOTONIC 1
int clock_gettime(int X, struct timespec* tv);
