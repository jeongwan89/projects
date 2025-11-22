#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief DHT22 센서 데이터 구조체
 */
typedef struct {
    float temperature;  // 온도 (°C)
    float humidity;     // 습도 (%)
    bool valid;         // 데이터 유효성
} dht22_data_t;

/**
 * @brief DHT22 센서 초기화
 * @param gpio_pin DHT22 데이터 핀 번호
 * @return 성공 시 true
 */
bool dht22_init(uint8_t gpio_pin);

/**
 * @brief DHT22 센서 데이터 읽기
 * @param data 읽은 데이터를 저장할 구조체 포인터
 * @return 성공 시 true
 */
bool dht22_read(dht22_data_t *data);

/**
 * @brief 온도만 빠르게 읽기
 * @return 온도 값 (실패 시 -999.0)
 */
float dht22_read_temperature(void);

/**
 * @brief 습도만 빠르게 읽기
 * @return 습도 값 (실패 시 -999.0)
 */
float dht22_read_humidity(void);

#endif // DHT22_H
