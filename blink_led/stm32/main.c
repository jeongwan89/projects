/**
 * @file main.c
 * @brief STM32 Blink LED 예제
 * @author jeongwan89
 * @date 2025
 */

#include "stm32f4xx_hal.h"

/* LED 핀 정의 */
#define LED_PIN GPIO_PIN_12
#define LED_PORT GPIOD

/* 함수 프로토타입 */
void SystemClock_Config(void);
void GPIO_Init(void);
void Error_Handler(void);

/**
 * @brief 메인 함수
 */
int main(void)
{
    /* HAL 라이브러리 초기화 */
    HAL_Init();
    
    /* 시스템 클럭 설정 */
    SystemClock_Config();
    
    /* GPIO 초기화 */
    GPIO_Init();
    
    /* 무한 루프 */
    while (1)
    {
        /* LED 토글 */
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        
        /* 500ms 지연 */
        HAL_Delay(500);
    }
}

/**
 * @brief GPIO 초기화 함수
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* GPIOD 클럭 활성화 */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    /* LED 핀 설정 */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

/**
 * @brief 시스템 클럭 설정 함수
 */
void SystemClock_Config(void)
{
    /* 시스템 클럭 설정 코드 (프로젝트에 맞게 수정 필요) */
}

/**
 * @brief 에러 핸들러
 */
void Error_Handler(void)
{
    /* 에러 발생 시 처리 */
    while (1)
    {
        /* 무한 루프 */
    }
}
