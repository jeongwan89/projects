#include "tm1637.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <cstring>

// TM1637 명령어
#define TM1637_CMD_DATA         0x40  // 데이터 명령 설정
#define TM1637_CMD_DISPLAY_CTRL 0x80  // 디스플레이 제어 명령
#define TM1637_CMD_ADDR         0xC0  // 주소 명령

// 타이밍 (마이크로초)
#define TM1637_BIT_DELAY        5     // 비트 간 지연 시간

// 7-세그먼트 숫자 패턴 (0-9)
// 비트 순서: .GFEDCBA
static const uint8_t digit_to_segment[10] = {
    0x3F, // 0: 0b00111111
    0x06, // 1: 0b00000110
    0x5B, // 2: 0b01011011
    0x4F, // 3: 0b01001111
    0x66, // 4: 0b01100110
    0x6D, // 5: 0b01101101
    0x7D, // 6: 0b01111101
    0x07, // 7: 0b00000111
    0x7F, // 8: 0b01111111
    0x6F  // 9: 0b01101111
};

// GPIO 핀 제어 (LOW-level 함수)
static inline void clk_high(TM1637Display& display) {
    gpio_set_dir(display.clk_pin, GPIO_IN);
    gpio_pull_up(display.clk_pin);
}

static inline void clk_low(TM1637Display& display) {
    gpio_set_dir(display.clk_pin, GPIO_OUT);
    gpio_put(display.clk_pin, 0);
}

static inline void dio_high(TM1637Display& display) {
    gpio_set_dir(display.dio_pin, GPIO_IN);
    gpio_pull_up(display.dio_pin);
}

static inline void dio_low(TM1637Display& display) {
    gpio_set_dir(display.dio_pin, GPIO_OUT);
    gpio_put(display.dio_pin, 0);
}

static inline bool dio_read(TM1637Display& display) {
    gpio_set_dir(display.dio_pin, GPIO_IN);
    return gpio_get(display.dio_pin);
}

static void tm1637_delay_us(uint32_t us) {
    sleep_us(us);
}

// I2C-like 시작 조건
static void tm1637_start(TM1637Display& display) {
    dio_high(display);
    clk_high(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
    dio_low(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
}

// I2C-like 정지 조건
static void tm1637_stop(TM1637Display& display) {
    dio_low(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
    clk_high(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
    dio_high(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
}

// 1바이트 전송
static bool tm1637_write_byte(TM1637Display& display, uint8_t data) {
    // 8비트 전송 (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        clk_low(display);
        tm1637_delay_us(TM1637_BIT_DELAY);
        
        if (data & 0x01) {
            dio_high(display);
        } else {
            dio_low(display);
        }
        tm1637_delay_us(TM1637_BIT_DELAY);
        
        clk_high(display);
        tm1637_delay_us(TM1637_BIT_DELAY);
        
        data >>= 1;
    }
    
    // ACK 비트 확인
    clk_low(display);
    dio_high(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
    
    clk_high(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
    bool ack = !dio_read(display);  // ACK는 LOW
    
    clk_low(display);
    tm1637_delay_us(TM1637_BIT_DELAY);
    
    return ack;
}

// 디스플레이 업데이트
static void tm1637_update_display(TM1637Display& display) {
    uint8_t ctrl = TM1637_CMD_DISPLAY_CTRL | (display.brightness & 0x07);
    if (display.display_on) {
        ctrl |= 0x08;  // Display ON
    }
    
    tm1637_start(display);
    tm1637_write_byte(display, ctrl);
    tm1637_stop(display);
}

bool tm1637_init(TM1637Display& display) {
    // GPIO 초기화
    gpio_init(display.clk_pin);
    gpio_init(display.dio_pin);
    
    gpio_set_dir(display.clk_pin, GPIO_OUT);
    gpio_set_dir(display.dio_pin, GPIO_OUT);
    
    gpio_put(display.clk_pin, 1);
    gpio_put(display.dio_pin, 1);
    
    // 기본 설정
    display.brightness = 7;
    display.display_on = true;
    
    // 디스플레이 클리어
    tm1637_clear(display);
    
    // 디스플레이 ON
    tm1637_update_display(display);
    
    return true;
}

void tm1637_set_brightness(TM1637Display& display, uint8_t brightness) {
    if (brightness > 7) brightness = 7;
    display.brightness = brightness;
    tm1637_update_display(display);
}

void tm1637_display_on(TM1637Display& display, bool on) {
    display.display_on = on;
    tm1637_update_display(display);
}

void tm1637_clear(TM1637Display& display) {
    uint8_t data[4] = {0, 0, 0, 0};
    
    // 데이터 명령 설정 (자동 주소 증가)
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_DATA);
    tm1637_stop(display);
    
    // 주소 0부터 4바이트 전송
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_ADDR);
    for (uint8_t i = 0; i < 4; i++) {
        tm1637_write_byte(display, data[i]);
    }
    tm1637_stop(display);
    
    tm1637_update_display(display);
}

void tm1637_show_segments(TM1637Display& display, uint8_t position, uint8_t segments) {
    if (position > 3) return;
    
    // 데이터 명령 설정 (고정 주소)
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_DATA | 0x04);
    tm1637_stop(display);
    
    // 특정 위치에 데이터 쓰기
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_ADDR | position);
    tm1637_write_byte(display, segments);
    tm1637_stop(display);
    
    tm1637_update_display(display);
}

void tm1637_show_number(TM1637Display& display, uint16_t number, bool show_leading_zero) {
    if (number > 9999) number = 9999;
    
    uint8_t digits[4];
    digits[0] = (number / 1000) % 10;
    digits[1] = (number / 100) % 10;
    digits[2] = (number / 10) % 10;
    digits[3] = number % 10;
    
    // 선행 0 처리
    bool started = show_leading_zero;
    for (uint8_t i = 0; i < 3; i++) {
        if (digits[i] != 0) started = true;
        if (!started) digits[i] = 0xFF;  // 빈칸
    }
    
    // 데이터 명령 설정
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_DATA);
    tm1637_stop(display);
    
    // 4자리 출력
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_ADDR);
    for (uint8_t i = 0; i < 4; i++) {
        if (digits[i] == 0xFF) {
            tm1637_write_byte(display, 0);  // 빈칸
        } else {
            tm1637_write_byte(display, digit_to_segment[digits[i]]);
        }
    }
    tm1637_stop(display);
    
    tm1637_update_display(display);
}

void tm1637_show_float(TM1637Display& display, float value, uint8_t decimal_places) {
    if (decimal_places > 3) decimal_places = 3;
    
    // 값의 범위 제한
    if (value < 0) value = 0;
    if (value > 9999) value = 9999;
    
    // 정수 변환
    int int_value = (int)(value * 10);  // 소수점 1자리까지만 (예: 25.3 → 253)
    
    uint8_t digits[4];
    digits[0] = (int_value / 100) % 10;
    digits[1] = (int_value / 10) % 10;
    digits[2] = int_value % 10;
    digits[3] = 0;  // 사용 안 함
    
    // 데이터 명령 설정
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_DATA);
    tm1637_stop(display);
    
    // 3자리 출력 + 소수점
    tm1637_start(display);
    tm1637_write_byte(display, TM1637_CMD_ADDR);
    
    // 첫 번째 자리 (십의 자리)
    tm1637_write_byte(display, digit_to_segment[digits[0]]);
    
    // 두 번째 자리 (일의 자리) + 소수점
    tm1637_write_byte(display, digit_to_segment[digits[1]] | 0x80);
    
    // 세 번째 자리 (소수점 첫째)
    tm1637_write_byte(display, digit_to_segment[digits[2]]);
    
    // 네 번째 자리 (빈칸)
    tm1637_write_byte(display, 0);
    
    tm1637_stop(display);
    
    tm1637_update_display(display);
}

void tm1637_show_temperature(TM1637Display& display, float temperature) {
    tm1637_show_float(display, temperature, 1);
}

void tm1637_show_humidity(TM1637Display& display, float humidity) {
    tm1637_show_float(display, humidity, 1);
}

void tm1637_show_colon(TM1637Display& display, bool show) {
    // TM1637 4-digit 모듈의 콜론은 보통 2번째 자리에 연결됨
    // 구현은 하드웨어에 따라 다를 수 있음
    // 여기서는 기본 구현만 제공
    if (show) {
        tm1637_show_segments(display, 1, digit_to_segment[0] | 0x80);
    }
}
