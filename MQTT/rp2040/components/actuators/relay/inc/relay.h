#ifndef RELAY_H
#define RELAY_H

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

void relay_init(uint pin);
void relay_on(uint pin);
void relay_off(uint pin);
void relay_toggle(uint pin);

#ifdef __cplusplus
}
#endif

#endif // RELAY_H
