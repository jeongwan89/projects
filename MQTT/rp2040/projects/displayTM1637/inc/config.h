#ifndef CONFIG_H
#define CONFIG_H

#include "mqtt_client.h"
#include "esp01.h"
#include "tm1637.h"
#include "hardware/uart.h"

// WiFi 설정
#define WIFI_SSID "FarmMain5G"
#define WIFI_PASSWORD "wweerrtt"

// ESP-01 하드웨어 설정
#define ESP01_UART uart1
#define ESP01_UART_TX_PIN 4
#define ESP01_UART_RX_PIN 5
#define ESP01_UART_BAUDRATE 115200
#define ESP01_RST_PIN 3

// TM1637 디스플레이 하드웨어 설정 (8개 병렬)
// GH1_TEMP, GH1_HUM, GH2_TEMP, GH2_HUM, GH3_TEMP, GH3_HUM, GH4_TEMP, GH4_HUM
#define NUM_DISPLAYS 8
#define TM1637_BRIGHTNESS 7 // 0-7

// 각 디스플레이 핀 설정 (CLK, DIO 쌍)
#define TM1637_GH1_TEMP_CLK 6
#define TM1637_GH1_TEMP_DIO 7
#define TM1637_GH1_HUM_CLK 20
#define TM1637_GH1_HUM_DIO 21
#define TM1637_GH2_TEMP_CLK 8
#define TM1637_GH2_TEMP_DIO 9
#define TM1637_GH2_HUM_CLK 18
#define TM1637_GH2_HUM_DIO 19
#define TM1637_GH3_TEMP_CLK 10
#define TM1637_GH3_TEMP_DIO 11
#define TM1637_GH3_HUM_CLK 16
#define TM1637_GH3_HUM_DIO 17
#define TM1637_GH4_TEMP_CLK 12
#define TM1637_GH4_TEMP_DIO 13
#define TM1637_GH4_HUM_CLK 14
#define TM1637_GH4_HUM_DIO 15

// MQTT 브로커 설정
#define MQTT_BROKER "192.168.0.24"
#define MQTT_PORT 1883
// Client ID는 main.cpp에서 RP2040 고유 ID를 사용해 동적 생성
#define MQTT_CLIENT_ID_PREFIX "rp2040_display_" // 고유 ID가 뒤에 붙음
#define MQTT_USERNAME "farmmain"
#define MQTT_PASSWORD "eerrtt"

// 연결 재확인 간격 (밀리초)
#define CONNECTION_CHECK_MS 5000 // 5초마다 연결 상태 확인
#define DISPLAY_UPDATE_MS 1000   // 1초마다 디스플레이 업데이트

// 센서 값 범위 검증 (온도/습도)
#define SENSOR_VALUE_MIN -99.9f
#define SENSOR_VALUE_MAX 99.9f

// MQTT 메시지 버퍼 크기
#define MQTT_TOPIC_MAX_LEN 64
#define MQTT_MESSAGE_MAX_LEN 128

// MQTT 토픽 - 디스플레이용
#define TOPIC_STATUS "Display/TM1637/status"
#define LWT_TOPIC TOPIC_STATUS
#define LWT_MESSAGE "offline"

// 센서 데이터 구독 토픽 (온실별 온도/습도)
// GH1~GH4: Greenhouse 1~4
#define TOPIC_GH1_TEMP "Sensor/GH1/Center/Temp"
#define TOPIC_GH1_HUM "Sensor/GH1/Center/Hum"
#define TOPIC_GH2_TEMP "Sensor/GH2/Center/Temp"
#define TOPIC_GH2_HUM "Sensor/GH2/Center/Hum"
#define TOPIC_GH3_TEMP "Sensor/GH3/Center/Temp"
#define TOPIC_GH3_HUM "Sensor/GH3/Center/Hum"
#define TOPIC_GH4_TEMP "Sensor/GH4/Center/Temp"
#define TOPIC_GH4_HUM "Sensor/GH4/Center/Hum"

// 보정값 구독 토픽 (온실별 온도/습도 오프셋)
#define TOPIC_GH1_TEMP_OFFSET "Calibration/GH1/Center/Temp_Offset"
#define TOPIC_GH1_HUM_OFFSET "Calibration/GH1/Center/Hum_Offset"
#define TOPIC_GH2_TEMP_OFFSET "Calibration/GH2/Center/Temp_Offset"
#define TOPIC_GH2_HUM_OFFSET "Calibration/GH2/Center/Hum_Offset"
#define TOPIC_GH3_TEMP_OFFSET "Calibration/GH3/Center/Temp_Offset"
#define TOPIC_GH3_HUM_OFFSET "Calibration/GH3/Center/Hum_Offset"
#define TOPIC_GH4_TEMP_OFFSET "Calibration/GH4/Center/Temp_Offset"
#define TOPIC_GH4_HUM_OFFSET "Calibration/GH4/Center/Hum_Offset"

// 디스플레이 인덱스
enum DisplayIndex
{
    DISPLAY_GH1_TEMP = 0,
    DISPLAY_GH1_HUM,
    DISPLAY_GH2_TEMP,
    DISPLAY_GH2_HUM,
    DISPLAY_GH3_TEMP,
    DISPLAY_GH3_HUM,
    DISPLAY_GH4_TEMP,
    DISPLAY_GH4_HUM
};

#endif // CONFIG_H
