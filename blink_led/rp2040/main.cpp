/**
 * @file main.cpp
 * @brief RP2040 Blink LED 예제
 * @author jeongwan89
 * @date 2025
 */

#include "pico/stdlib.h"

/* 내장 LED 핀 (Pico 보드 기준 GPIO25) */
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

/**
 * @brief 메인 함수
 */
int main()
{
    /* 표준 라이브러리 초기화 */
    stdio_init_all();

    /* LED 핀 초기화 */
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    /* 무한 루프 */
    while (true)
    {
        /* LED 켜기 */
        gpio_put(LED_PIN, 1);
        sleep_ms(500);

        /* LED 끄기 */
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }

    return 0;
}
