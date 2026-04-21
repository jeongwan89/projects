#include "ultrasonic.h"

#include "hardware/gpio.h"

#define ULTRASONIC_DEFAULT_TIMEOUT_US 30000u

static bool wait_for_level(uint pin, bool level, uint32_t timeout_us)
{
    uint32_t start = time_us_32();

    while (gpio_get(pin) != level) {
        if ((time_us_32() - start) > timeout_us) {
            return false;
        }
    }

    return true;
}

bool ultrasonic_init(ultrasonic_sensor_t *sensor, uint trig_pin, uint echo_pin)
{
    if (sensor == NULL) {
        return false;
    }

    sensor->trig_pin = trig_pin;
    sensor->echo_pin = echo_pin;
    sensor->echo_timeout_us = ULTRASONIC_DEFAULT_TIMEOUT_US;

    gpio_init(trig_pin);
    gpio_set_dir(trig_pin, GPIO_OUT);
    gpio_put(trig_pin, 0);

    gpio_init(echo_pin);
    gpio_set_dir(echo_pin, GPIO_IN);

    return true;
}

bool ultrasonic_read_cm(const ultrasonic_sensor_t *sensor, float *distance_cm)
{
    uint32_t pulse_start;
    uint32_t pulse_end;

    if (sensor == NULL || distance_cm == NULL) {
        return false;
    }

    gpio_put(sensor->trig_pin, 0);
    sleep_us(2);
    gpio_put(sensor->trig_pin, 1);
    sleep_us(10);
    gpio_put(sensor->trig_pin, 0);

    if (!wait_for_level(sensor->echo_pin, true, sensor->echo_timeout_us)) {
        return false;
    }

    pulse_start = time_us_32();

    if (!wait_for_level(sensor->echo_pin, false, sensor->echo_timeout_us)) {
        return false;
    }

    pulse_end = time_us_32();

    *distance_cm = (float)(pulse_end - pulse_start) / 58.0f;
    return true;
}
