#pragma once
#include <string.h>

//Code from RetroArch (MIT license)

#ifdef _MSC_VER
#undef strcasecmp
#define strcasecmp(a, b) retro_strcasecmp__(a, b)
int strcasecmp(const char *a, const char *b);
#endif
