/**
 * @file main.c
 * @brief ESP32 Blink LED 예제 (ESP-IDF)
 * @author jeongwan89
 * @date 2025
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

/* LED 핀 정의 */
#define LED_GPIO GPIO_NUM_2
#define BLINK_PERIOD_MS 500

/* 로그 태그 */
static const char *TAG = "BLINK";

/**
 * @brief LED 깜빡임 태스크
 */
void blink_task(void *pvParameter)
{
    /* GPIO 설정 */
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    
    ESP_LOGI(TAG, "ESP32 Blink LED 시작");
    
    uint8_t led_state = 0;
    
    while (1) 
    {
        /* LED 상태 토글 */
        led_state = !led_state;
        gpio_set_level(LED_GPIO, led_state);
        
        ESP_LOGI(TAG, "LED: %s", led_state ? "ON" : "OFF");
        
        /* 지연 */
        vTaskDelay(BLINK_PERIOD_MS / portTICK_PERIOD_MS);
    }
}

/**
 * @brief 애플리케이션 메인 함수
 */
void app_main(void)
{
    /* Blink 태스크 생성 */
    xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);
}
