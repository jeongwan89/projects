#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define DS18B20_PIN 16

// 실제로는 1-Wire 프로토콜 구현 필요. 여기서는 예시로 핀 토글만 반복.
int main() {
    stdio_init_all();
    gpio_init(DS18B20_PIN);
    gpio_set_dir(DS18B20_PIN, GPIO_OUT);
    printf("\n=== DS18B20 센서 예제 (RP2040) ===\n");
    while (true) {
        gpio_put(DS18B20_PIN, 1);
        printf("DS18B20: HIGH\n");
        sleep_ms(500);
        gpio_put(DS18B20_PIN, 0);
        printf("DS18B20: LOW\n");
        sleep_ms(500);
    }
    return 0;
}
