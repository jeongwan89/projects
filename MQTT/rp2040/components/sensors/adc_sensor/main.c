#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_PIN 26  // GPIO26 = ADC0

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0); // ADC0
    printf("\n=== ADC 센서 예제 (RP2040) ===\n");
    while (true) {
        uint16_t raw = adc_read();
        float voltage = raw * 3.3f / 4095.0f;
        printf("ADC0: %u, %.3f V\n", raw, voltage);
        sleep_ms(1000);
    }
    return 0;
}
