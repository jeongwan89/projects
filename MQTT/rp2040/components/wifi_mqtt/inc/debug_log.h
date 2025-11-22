#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <stdio.h>

#define DEBUG_LOG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)

#endif // DEBUG_LOG_H
