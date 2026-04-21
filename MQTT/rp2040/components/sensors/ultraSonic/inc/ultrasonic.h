#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint trig_pin;
    uint echo_pin;
    uint32_t echo_timeout_us;
} ultrasonic_sensor_t;

bool ultrasonic_init(ultrasonic_sensor_t *sensor, uint trig_pin, uint echo_pin);
bool ultrasonic_read_cm(const ultrasonic_sensor_t *sensor, float *distance_cm);

#ifdef __cplusplus
}
#endif

#endif // ULTRASONIC_H
