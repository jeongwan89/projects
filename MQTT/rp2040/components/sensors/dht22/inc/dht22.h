#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

bool dht22_read(uint pin, float* temperature, float* humidity);

#ifdef __cplusplus
}
#endif

#endif // DHT22_H
