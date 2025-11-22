#include "dht22.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>

static uint8_t dht_pin = 0;
static bool initialized = false;

bool dht22_init(uint8_t gpio_pin) {
    dht_pin = gpio_pin;
    gpio_init(dht_pin);
    initialized = true;
    return true;
}

bool dht22_read(dht22_data_t *data) {
    if (!initialized || data == NULL) {
        return false;
    }
    
    uint8_t bits[5] = {0};
    uint8_t bit_idx = 0;
    uint8_t byte_idx = 0;
    
    // DHT22 시작 신호 전송
    gpio_set_dir(dht_pin, GPIO_OUT);
    gpio_put(dht_pin, 0);
    sleep_ms(1);  // 최소 1ms LOW
    gpio_put(dht_pin, 1);
    sleep_us(30);
    
    // 입력 모드로 전환
    gpio_set_dir(dht_pin, GPIO_IN);
    
    // DHT22 응답 대기 (80us LOW + 80us HIGH)
    uint32_t timeout = 100;
    while (gpio_get(dht_pin) == 1 && timeout--) sleep_us(1);
    if (timeout == 0) goto error;
    
    timeout = 100;
    while (gpio_get(dht_pin) == 0 && timeout--) sleep_us(1);
    if (timeout == 0) goto error;
    
    timeout = 100;
    while (gpio_get(dht_pin) == 1 && timeout--) sleep_us(1);
    if (timeout == 0) goto error;
    
    // 데이터 비트 읽기 (40비트)
    for (int i = 0; i < 40; i++) {
        // 각 비트 시작: 50us LOW
        timeout = 100;
        while (gpio_get(dht_pin) == 0 && timeout--) sleep_us(1);
        if (timeout == 0) goto error;
        
        // HIGH 지속시간으로 비트 판별 (26-28us=0, 70us=1)
        uint32_t high_time = 0;
        while (gpio_get(dht_pin) == 1 && high_time < 100) {
            sleep_us(1);
            high_time++;
        }
        
        bits[byte_idx] <<= 1;
        if (high_time > 40) {  // 1 비트
            bits[byte_idx] |= 1;
        }
        
        if (++bit_idx == 8) {
            bit_idx = 0;
            byte_idx++;
        }
    }
    
    // 체크섬 검증
    uint8_t checksum = bits[0] + bits[1] + bits[2] + bits[3];
    if (checksum != bits[4]) {
        goto error;
    }
    
    // 데이터 변환
    data->humidity = ((bits[0] << 8) | bits[1]) / 10.0f;
    data->temperature = (((bits[2] & 0x7F) << 8) | bits[3]) / 10.0f;
    if (bits[2] & 0x80) {  // 음수 온도
        data->temperature = -data->temperature;
    }
    data->valid = true;
    
    return true;

error:
    data->valid = false;
    data->temperature = -999.0f;
    data->humidity = -999.0f;
    return false;
}

float dht22_read_temperature(void) {
    dht22_data_t data;
    if (dht22_read(&data)) {
        return data.temperature;
    }
    return -999.0f;
}

float dht22_read_humidity(void) {
    dht22_data_t data;
    if (dht22_read(&data)) {
        return data.humidity;
    }
    return -999.0f;
}
