/**
 * @file blink_led.ino
 * @brief ESP32 Blink LED 예제 (Arduino Framework)
 * @author jeongwan89
 * @date 2025
 */

/* LED 핀 정의 */
const int LED_PIN = 2;  // ESP32 내장 LED (GPIO2)

/**
 * @brief 초기화 함수 (1회 실행)
 */
void setup() 
{
    // LED 핀을 출력으로 설정
    pinMode(LED_PIN, OUTPUT);
    
    // 시리얼 통신 초기화 (디버깅용)
    Serial.begin(115200);
    Serial.println("ESP32 Blink LED 시작");
}

/**
 * @brief 메인 루프 함수 (반복 실행)
 */
void loop() 
{
    // LED 켜기
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
    
    // 500ms 대기
    delay(500);
    
    // LED 끄기
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
    
    // 500ms 대기
    delay(500);
}
