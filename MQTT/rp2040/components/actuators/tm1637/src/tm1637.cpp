#include "tm1637.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// 7-세그먼트 숫자 패턴 (0-9)
//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D

// 비트 순서: .GFEDCBA
static const uint8_t DIGIT_TO_SEGMENT[16] = {
    0x3F, // 0: 0b00111111
    0x06, // 1: 0b00000110
    0x5B, // 2: 0b01011011
    0x4F, // 3: 0b01001111
    0x66, // 4: 0b01100110
    0x6D, // 5: 0b01101101
    0x7D, // 6: 0b01111101
    0x07, // 7: 0b00000111
    0x7F, // 8: 0b01111111
    0x6F, // 9: 0b01101111
    0x77, // A: 0b01110111
    0x7C, // b: 0b01111100
    0x39, // C: 0b00111001
    0x5E, // d: 0b01011110
    0x79, // E: 0b01111001
    0x71  // F: 0b01110001
};

// ========== 생성자 ==========
TM1637Display::TM1637Display(uint8_t clk_pin, uint8_t dio_pin)
    : clk_pin_(clk_pin)
    , dio_pin_(dio_pin)
    , brightness_(7)
    , display_on_(true)
{
}

// ========== Public 메서드 ==========
bool TM1637Display::init() {
    // GPIO 초기화
    gpio_init(clk_pin_);
    gpio_init(dio_pin_);
    
    // Pull-up 설정 (오픈 드레인 통신)
    gpio_pull_up(clk_pin_);
    gpio_pull_up(dio_pin_);
    
    // 초기 상태: HIGH (idle)
    gpio_set_dir(clk_pin_, GPIO_OUT);
    gpio_set_dir(dio_pin_, GPIO_OUT);
    gpio_put(clk_pin_, 1);
    gpio_put(dio_pin_, 1);
    
    // 안정화 대기
    sleep_ms(10);
    
    // 기본 설정
    brightness_ = 7;
    display_on_ = true;
    
    // 통신 테스트: 디스플레이 제어 명령 전송
    start();
    bool ack = writeByte(CMD_DISPLAY_CTRL | 0x08 | 0x07);
    stop();
    
    if (!ack) {
        return false;  // ACK 실패 = 통신 실패
    }
    
    // 디스플레이 클리어
    clear();
    
    // 디스플레이 ON
    updateDisplay();
    
    return true;
}

void TM1637Display::setBrightness(uint8_t brightness) {
    if (brightness > 7) brightness = 7;
    brightness_ = brightness;
    updateDisplay();
}

void TM1637Display::displayOn(bool on) {
    display_on_ = on;
    updateDisplay();
}

void TM1637Display::clear() {
    uint8_t data[4] = {0, 0, 0, 0};
    
    // 데이터 명령 설정 (자동 주소 증가)
    start();
    writeByte(CMD_DATA);
    stop();
    
    // 주소 0부터 4바이트 전송
    start();
    writeByte(CMD_ADDR);
    for (uint8_t i = 0; i < 4; i++) {
        writeByte(data[i]);
    }
    stop();
    
    updateDisplay();
}

void TM1637Display::showSegments(uint8_t position, uint8_t segments) {
    if (position > 3) return;
    
    // 데이터 명령 설정 (고정 주소)
    start();
    writeByte(CMD_DATA | 0x04);
    stop();
    
    // 특정 위치에 데이터 쓰기
    start();
    writeByte(CMD_ADDR | position);
    writeByte(segments);
    stop();
    
    updateDisplay();
}

void TM1637Display::showNumber(uint16_t number, bool show_leading_zero) {
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
    start();
    writeByte(CMD_DATA);
    stop();
    
    // 4자리 출력
    start();
    writeByte(CMD_ADDR);
    for (uint8_t i = 0; i < 4; i++) {
        if (digits[i] == 0xFF) {
            writeByte(0);  // 빈칸
        } else {
            writeByte(digitToSegment(digits[i]));
        }
    }
    stop();
    
    updateDisplay();
}

void TM1637Display::showFloat(float value, uint8_t decimal_places) {
    if (decimal_places > 3) decimal_places = 3;
    
    // 값의 범위 제한
    if (value < 0) value = 0;
    if (value > 999.9f) value = 999.9f;
    
    // 소수점 1자리 고정으로 정수 변환 (예: 25.3 → 253)
    int int_value = (int)(value * 10);
    
    uint8_t digits[4];
    
    // 오른쪽 맞춤: 자릿수 추출
    digits[3] = int_value % 10;           // 소수점 첫째 자리
    digits[2] = (int_value / 10) % 10;    // 일의 자리 (소수점 위치)
    digits[1] = (int_value / 100) % 10;   // 십의 자리
    digits[0] = (int_value / 1000) % 10;  // 백의 자리
    
    // 선행 0 처리 (백의 자리, 십의 자리)
    bool started = false;
    for (uint8_t i = 0; i < 2; i++) {
        if (digits[i] != 0) started = true;
        if (!started) digits[i] = 0xFF;  // 빈칸 표시
    }
    
    // 데이터 명령 설정
    start();
    writeByte(CMD_DATA);
    stop();
    
    // 4자리 출력 (오른쪽 맞춤)
    start();
    writeByte(CMD_ADDR);
    
    // 첫 번째 자리 (백의 자리 또는 빈칸)
    if (digits[0] == 0xFF) {
        writeByte(0);
    } else {
        writeByte(digitToSegment(digits[0]));
    }
    
    // 두 번째 자리 (십의 자리 또는 빈칸)
    if (digits[1] == 0xFF) {
        writeByte(0);
    } else {
        writeByte(digitToSegment(digits[1]));
    }
    
    // 세 번째 자리 (일의 자리 + 소수점)
    writeByte(digitToSegment(digits[2]) | 0x80);
    
    // 네 번째 자리 (소수점 첫째 자리)
    writeByte(digitToSegment(digits[3]));
    
    stop();
    
    updateDisplay();
}

void TM1637Display::showTemperature(float temperature) {
    showFloat(temperature, 1);
}

void TM1637Display::showHumidity(float humidity) {
    showFloat(humidity, 1);
}

void TM1637Display::showColon(bool show) {
    // TM1637 4-digit 모듈의 콜론은 보통 2번째 자리에 연결됨
    // 구현은 하드웨어에 따라 다를 수 있음
    // 여기서는 기본 구현만 제공
    if (show) {
        showSegments(1, digitToSegment(0) | 0x80);
    }
}

// ========== Private 메서드 (GPIO 제어) ==========
void TM1637Display::clkHigh() {
    gpio_set_dir(clk_pin_, GPIO_IN);
    gpio_pull_up(clk_pin_);
}

void TM1637Display::clkLow() {
    gpio_set_dir(clk_pin_, GPIO_OUT);
    gpio_put(clk_pin_, 0);
}

void TM1637Display::dioHigh() {
    gpio_set_dir(dio_pin_, GPIO_IN);
    gpio_pull_up(dio_pin_);
}

void TM1637Display::dioLow() {
    gpio_set_dir(dio_pin_, GPIO_OUT);
    gpio_put(dio_pin_, 0);
}

bool TM1637Display::dioRead() {
    gpio_set_dir(dio_pin_, GPIO_IN);
    return gpio_get(dio_pin_);
}

// ========== Private 메서드 (통신 프로토콜) ==========
void TM1637Display::start() {
    dioHigh();
    clkHigh();
    delayUs(BIT_DELAY);
    dioLow();
    delayUs(BIT_DELAY);
}

void TM1637Display::stop() {
    dioLow();
    delayUs(BIT_DELAY);
    clkHigh();
    delayUs(BIT_DELAY);
    dioHigh();
    delayUs(BIT_DELAY);
}

bool TM1637Display::writeByte(uint8_t data) {
    // 8비트 전송 (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        clkLow();
        delayUs(BIT_DELAY);
        
        if (data & 0x01) {
            dioHigh();
        } else {
            dioLow();
        }
        delayUs(BIT_DELAY);
        
        clkHigh();
        delayUs(BIT_DELAY);
        
        data >>= 1;
    }
    
    // ACK 비트 확인
    clkLow();
    dioHigh();
    delayUs(BIT_DELAY);
    
    clkHigh();
    delayUs(BIT_DELAY);
    bool ack = !dioRead();  // ACK는 LOW
    
    clkLow();
    delayUs(BIT_DELAY);
    
    return ack;
}

void TM1637Display::updateDisplay() {
    uint8_t ctrl = CMD_DISPLAY_CTRL | (brightness_ & 0x07);
    if (display_on_) {
        ctrl |= 0x08;  // Display ON
    }
    
    start();
    writeByte(ctrl);
    stop();
}

// ========== Private 정적 메서드 (유틸리티) ==========
void TM1637Display::delayUs(uint32_t us) {
    sleep_us(us);
}

uint8_t TM1637Display::digitToSegment(uint8_t digit) {
    if (digit > 15) return 0;
    return DIGIT_TO_SEGMENT[digit];
}
