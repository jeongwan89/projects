#ifndef TM1637_H
#define TM1637_H

#include <cstdint>

/**
 * @brief TM1637 디스플레이 클래스
 * 
 * TM1637은 CLK, DIO 2선식 통신을 사용하는 4-digit 7-segment LED 디스플레이입니다.
 * 이 클래스는 TM1637 모듈을 제어하기 위한 모든 기능을 제공합니다.
 */
class TM1637Display {
public:
    /**
     * @brief TM1637 디스플레이 생성자
     * 
     * @param clk_pin 클럭 핀 번호
     * @param dio_pin 데이터 핀 번호
     */
    TM1637Display(uint8_t clk_pin, uint8_t dio_pin);
    
    /**
     * @brief 디스플레이 초기화
     * 
     * @return true 초기화 성공
     * @return false 초기화 실패
     */
    bool init();
    
    /**
     * @brief 디스플레이 밝기 설정
     * 
     * @param brightness 밝기 (0-7, 0=가장 어두움, 7=가장 밝음)
     */
    void setBrightness(uint8_t brightness);
    
    /**
     * @brief 디스플레이 ON/OFF
     * 
     * @param on true=켜기, false=끄기
     */
    void displayOn(bool on);
    
    /**
     * @brief 전체 디스플레이 클리어
     */
    void clear();
    
    /**
     * @brief 4자리 숫자 표시 (0000-9999)
     * 
     * @param number 표시할 숫자 (0-9999)
     * @param show_leading_zero true=선행 0 표시, false=선행 0 숨김
     */
    void showNumber(uint16_t number, bool show_leading_zero = false);
    
    /**
     * @brief 소수점이 있는 숫자 표시 (예: 12.34)
     * 
     * @param value 표시할 값
     * @param decimal_places 소수점 자릿수 (0-3)
     */
    void showFloat(float value, uint8_t decimal_places = 1);
    
    /**
     * @brief 온도 표시 (예: 25.3°C → "25.3" + 우측 점)
     * 
     * @param temperature 온도 값
     */
    void showTemperature(float temperature);
    
    /**
     * @brief 습도 표시 (예: 65.5% → "65.5")
     * 
     * @param humidity 습도 값
     */
    void showHumidity(float humidity);
    
    /**
     * @brief 특정 위치에 세그먼트 데이터 직접 표시
     * 
     * @param position 위치 (0-3, 0=가장 왼쪽)
     * @param segments 세그먼트 데이터 (비트 패턴)
     */
    void showSegments(uint8_t position, uint8_t segments);
    
    /**
     * @brief 콜론(:) 표시/숨김 (시간 표시용)
     * 
     * @param show true=표시, false=숨김
     */
    void showColon(bool show);

private:
    // TM1637 명령어
    static constexpr uint8_t CMD_DATA = 0x40;         // 데이터 명령 설정
    static constexpr uint8_t CMD_DISPLAY_CTRL = 0x80; // 디스플레이 제어 명령
    static constexpr uint8_t CMD_ADDR = 0xC0;         // 주소 명령
    
    // 타이밍 (마이크로초) - 일부 모듈은 더 긴 지연 필요
    static constexpr uint32_t BIT_DELAY = 100;        // 비트 간 지연 시간 (5us → 100us로 증가)
    
    // 멤버 변수
    uint8_t clk_pin_;        // 클럭 핀 번호
    uint8_t dio_pin_;        // 데이터 핀 번호
    uint8_t brightness_;     // 밝기 (0-7)
    bool display_on_;        // 디스플레이 ON/OFF 상태
    
    // LOW-level GPIO 제어 함수
    void clkHigh();
    void clkLow();
    void dioHigh();
    void dioLow();
    bool dioRead();
    
    // 통신 프로토콜 함수
    void start();
    void stop();
    bool writeByte(uint8_t data);
    void updateDisplay();
    
    // 유틸리티 함수
    static void delayUs(uint32_t us);
    static uint8_t digitToSegment(uint8_t digit);
};

#endif // TM1637_H
