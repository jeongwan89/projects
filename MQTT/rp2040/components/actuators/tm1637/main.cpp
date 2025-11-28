#include <stdio.h>
#include "pico/stdlib.h"
#include "tm1637.h"

// 테스트용 핀 설정 (변경 가능)
#define TEST_CLK_PIN 27
#define TEST_DIO_PIN 26

int main() {
    // 표준 입출력 초기화
    stdio_init_all();
    
    // USB CDC 연결 대기 (최대 5초)
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while (!stdio_usb_connected() && (to_ms_since_boot(get_absolute_time()) - start < 5000)) {
        sleep_ms(100);
    }
    sleep_ms(500);
    
    printf("\n\n=== TM1637 단독 테스트 프로그램 ===\n");
    printf("CLK 핀: GPIO %d\n", TEST_CLK_PIN);
    printf("DIO 핀: GPIO %d\n\n", TEST_DIO_PIN);
    
    // TM1637 디스플레이 생성 및 초기화
    TM1637Display display(TEST_CLK_PIN, TEST_DIO_PIN);
    
    if (!display.init()) {
        printf("[오류] TM1637 초기화 실패!\n");
        printf("연결을 확인하세요:\n");
        printf("  - CLK: GPIO %d\n", TEST_CLK_PIN);
        printf("  - DIO: GPIO %d\n", TEST_DIO_PIN);
        printf("  - VCC: 3.3V 또는 5V\n");
        printf("  - GND: GND\n");
        return -1;
    }
    
    printf("[OK] TM1637 초기화 성공!\n\n");
    
    // 밝기 설정
    display.setBrightness(7);  // 최대 밝기
    printf("밝기 설정: 7 (최대)\n\n");
    
    // 테스트 1: 모든 세그먼트 켜기 (8888)
    printf("테스트 1: 8888 표시 (2초)...\n");
    display.showNumber(8888, true);
    sleep_ms(2000);
    
    // 테스트 2: 화면 클리어
    printf("테스트 2: 화면 클리어 (1초)...\n");
    display.clear();
    sleep_ms(1000);
    
    // 테스트 3: 숫자 카운트 (0-9999)
    printf("테스트 3: 숫자 카운트 (0-99)...\n");
    for (uint16_t i = 0; i <= 99; i++) {
        display.showNumber(i, false);
        sleep_ms(50);
    }
    sleep_ms(1000);
    
    // 테스트 4: 소수점 있는 숫자
    printf("테스트 4: 소수점 숫자 (12.3)...\n");
    display.showFloat(12.3f, 1);
    sleep_ms(2000);
    
    // 테스트 5: 온도 표시
    printf("테스트 5: 온도 표시 (25.6°C)...\n");
    display.showTemperature(25.6f);
    sleep_ms(2000);
    
    // 테스트 6: 습도 표시
    printf("테스트 6: 습도 표시 (65.5%%)...\n");
    display.showHumidity(65.5f);
    sleep_ms(2000);
    
    // 테스트 7: 밝기 변경 테스트
    printf("테스트 7: 밝기 변경 (0-7)...\n");
    display.showNumber(8888, true);
    for (uint8_t brightness = 0; brightness <= 7; brightness++) {
        printf("  밝기: %d\n", brightness);
        display.setBrightness(brightness);
        sleep_ms(500);
    }
    display.setBrightness(7);  // 다시 최대 밝기로
    sleep_ms(1000);
    
    // 테스트 8: ON/OFF 테스트
    printf("테스트 8: ON/OFF 토글...\n");
    for (int i = 0; i < 5; i++) {
        printf("  OFF\n");
        display.displayOn(false);
        sleep_ms(500);
        printf("  ON\n");
        display.displayOn(true);
        sleep_ms(500);
    }
    
    // 테스트 9: 개별 세그먼트 테스트
    printf("테스트 9: 개별 세그먼트 테스트...\n");
    display.clear();
    sleep_ms(500);
    
    // 각 자리에 차례로 숫자 표시
    for (uint8_t pos = 0; pos < 4; pos++) {
        printf("  위치 %d: 숫자 %d\n", pos, pos);
        display.showSegments(pos, 0x3F);  // 숫자 0 패턴
        sleep_ms(500);
    }
    sleep_ms(1000);
    
    printf("\n=== 모든 테스트 완료! ===\n");
    printf("무한 루프: 온도/습도 시뮬레이션 시작...\n\n");
    
    // 무한 루프: 온도와 습도를 번갈아 표시
    float temperature = 20.0f;
    float humidity = 50.0f;
    bool show_temp = true;
    
    while (true) {
        if (show_temp) {
            printf("온도: %.1f°C\n", temperature);
            display.showTemperature(temperature);
            
            // 온도 변화 시뮬레이션 (20.0 ~ 30.0)
            temperature += 0.1f;
            if (temperature > 30.0f) temperature = 20.0f;
        } else {
            printf("습도: %.1f%%\n", humidity);
            display.showHumidity(humidity);
            
            // 습도 변화 시뮬레이션 (40.0 ~ 80.0)
            humidity += 0.5f;
            if (humidity > 80.0f) humidity = 40.0f;
        }
        
        show_temp = !show_temp;
        sleep_ms(2000);
    }
    
    return 0;
}
