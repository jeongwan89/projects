#ifndef TM1637_H
#define TM1637_H

#include <cstdint>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TM1637 디스플레이 모듈 구조체
 * 
 * TM1637은 CLK, DIO 2선식 통신을 사용하는 4-digit 7-segment LED 디스플레이입니다.
 */
typedef struct {
    uint8_t clk_pin;     // 클럭 핀 번호
    uint8_t dio_pin;     // 데이터 핀 번호
    uint8_t brightness;  // 밝기 (0-7, 0=가장 어두움, 7=가장 밝음)
    bool display_on;     // 디스플레이 ON/OFF 상태
} TM1637Display;

/**
 * @brief TM1637 디스플레이 초기화
 * 
 * @param display TM1637 디스플레이 구조체
 * @return true 초기화 성공
 * @return false 초기화 실패
 */
bool tm1637_init(TM1637Display& display);

/**
 * @brief 디스플레이 밝기 설정
 * 
 * @param display TM1637 디스플레이 구조체
 * @param brightness 밝기 (0-7)
 */
void tm1637_set_brightness(TM1637Display& display, uint8_t brightness);

/**
 * @brief 디스플레이 ON/OFF
 * 
 * @param display TM1637 디스플레이 구조체
 * @param on true=켜기, false=끄기
 */
void tm1637_display_on(TM1637Display& display, bool on);

/**
 * @brief 전체 디스플레이 클리어
 * 
 * @param display TM1637 디스플레이 구조체
 */
void tm1637_clear(TM1637Display& display);

/**
 * @brief 4자리 숫자 표시 (0000-9999)
 * 
 * @param display TM1637 디스플레이 구조체
 * @param number 표시할 숫자 (0-9999)
 * @param show_leading_zero true=선행 0 표시, false=선행 0 숨김
 */
void tm1637_show_number(TM1637Display& display, uint16_t number, bool show_leading_zero);

/**
 * @brief 소수점이 있는 숫자 표시 (예: 12.34)
 * 
 * @param display TM1637 디스플레이 구조체
 * @param value 표시할 값
 * @param decimal_places 소수점 자릿수 (0-3)
 */
void tm1637_show_float(TM1637Display& display, float value, uint8_t decimal_places);

/**
 * @brief 온도 표시 (예: 25.3°C → "25.3" + 우측 점)
 * 
 * @param display TM1637 디스플레이 구조체
 * @param temperature 온도 값
 */
void tm1637_show_temperature(TM1637Display& display, float temperature);

/**
 * @brief 습도 표시 (예: 65.5% → "65.5")
 * 
 * @param display TM1637 디스플레이 구조체
 * @param humidity 습도 값
 */
void tm1637_show_humidity(TM1637Display& display, float humidity);

/**
 * @brief 특정 위치에 세그먼트 데이터 직접 표시
 * 
 * @param display TM1637 디스플레이 구조체
 * @param position 위치 (0-3, 0=가장 왼쪽)
 * @param segments 세그먼트 데이터 (비트 패턴)
 */
void tm1637_show_segments(TM1637Display& display, uint8_t position, uint8_t segments);

/**
 * @brief 콜론(:) 표시/숨김 (시간 표시용)
 * 
 * @param display TM1637 디스플레이 구조체
 * @param show true=표시, false=숨김
 */
void tm1637_show_colon(TM1637Display& display, bool show);

#ifdef __cplusplus
}
#endif

#endif // TM1637_H
