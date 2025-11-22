#include "relay.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

bool relay_init(uint8_t gpio_pin) {
    gpio_init(gpio_pin);
    gpio_set_dir(gpio_pin, GPIO_OUT);
    gpio_put(gpio_pin, 0);  // 초기 상태: OFF
    return true;
}

void relay_on(uint8_t gpio_pin) {
    gpio_put(gpio_pin, 1);
}

void relay_off(uint8_t gpio_pin) {
    gpio_put(gpio_pin, 0);
}

void relay_toggle(uint8_t gpio_pin) {
    gpio_put(gpio_pin, !gpio_get(gpio_pin));
}

bool relay_get_state(uint8_t gpio_pin) {
    return gpio_get(gpio_pin);
}

void relay_set(uint8_t gpio_pin, bool state) {
    gpio_put(gpio_pin, state ? 1 : 0);
}
