#ifndef RELAY_H
#define RELAY_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 릴레이 초기화
 * @param gpio_pin 릴레이 제어 핀 번호
 * @return 성공 시 true
 */
bool relay_init(uint8_t gpio_pin);

/**
 * @brief 릴레이 켜기
 * @param gpio_pin 릴레이 제어 핀 번호
 */
void relay_on(uint8_t gpio_pin);

/**
 * @brief 릴레이 끄기
 * @param gpio_pin 릴레이 제어 핀 번호
 */
void relay_off(uint8_t gpio_pin);

/**
 * @brief 릴레이 토글
 * @param gpio_pin 릴레이 제어 핀 번호
 */
void relay_toggle(uint8_t gpio_pin);

/**
 * @brief 릴레이 상태 읽기
 * @param gpio_pin 릴레이 제어 핀 번호
 * @return ON이면 true, OFF면 false
 */
bool relay_get_state(uint8_t gpio_pin);

/**
 * @brief 릴레이 설정 (ON/OFF)
 * @param gpio_pin 릴레이 제어 핀 번호
 * @param state true=ON, false=OFF
 */
void relay_set(uint8_t gpio_pin, bool state);

#endif // RELAY_H
