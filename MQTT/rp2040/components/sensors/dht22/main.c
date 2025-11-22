#include <stdio.h>
#include "pico/stdlib.h"
#include "dht22.h"

#define DHT22_PIN 15

int main() {
    stdio_init_all();
    dht22_init(DHT22_PIN);
    printf("\n=== DHT22 센서 예제 (RP2040) ===\n");
    while (true) {
        dht22_data_t data;
        if (dht22_read(&data) && data.valid) {
            printf("온도: %.1f°C, 습도: %.1f%%\n", data.temperature, data.humidity);
        } else {
            printf("센서 읽기 실패\n");
        }
        sleep_ms(2000);
    }
    return 0;
}
