#ifndef CONFIG_H
#define CONFIG_H

#include "mqtt_client.h"
#include "esp01.h"
#include "tm1637.h"
#include "hardware/uart.h"

// WiFi 설정
#define WIFI_SSID     "FarmMain5G"
#define WIFI_PASSWORD "wweerrtt"

// ESP-01 하드웨어 설정
#define ESP01_UART          uart1
#define ESP01_UART_TX_PIN   4
#define ESP01_UART_RX_PIN   5
#define ESP01_UART_BAUDRATE 115200
#define ESP01_RST_PIN       3

// TM1637 디스플레이 하드웨어 설정
#define TM1637_CLK_PIN      10
#define TM1637_DIO_PIN      11
#define TM1637_BRIGHTNESS   7   // 0-7

// MQTT 브로커 설정
#define MQTT_BROKER     "192.168.0.24"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "rp2040_display_tm1637_001"
#define MQTT_USERNAME   "farmmain"
#define MQTT_PASSWORD   "eerrtt"

// MQTT 토픽 - 디스플레이용
#define TOPIC_STATUS    "Display/TM1637/status"
#define LWT_TOPIC       TOPIC_STATUS
#define LWT_MESSAGE     "offline"

// 센서 데이터 구독 토픽 (온실별 온도/습도)
// GH1~GH4: Greenhouse 1~4
#define TOPIC_GH1_TEMP  "Sensor/GH1/Center/Temp"
#define TOPIC_GH1_HUM   "Sensor/GH1/Center/Hum"
#define TOPIC_GH2_TEMP  "Sensor/GH2/Center/Temp"
#define TOPIC_GH2_HUM   "Sensor/GH2/Center/Hum"
#define TOPIC_GH3_TEMP  "Sensor/GH3/Center/Temp"
#define TOPIC_GH3_HUM   "Sensor/GH3/Center/Hum"
#define TOPIC_GH4_TEMP  "Sensor/GH4/Center/Temp"
#define TOPIC_GH4_HUM   "Sensor/GH4/Center/Hum"

// 디스플레이 모드 제어 토픽
#define TOPIC_DISPLAY_MODE  "Display/TM1637/mode"

// 디스플레이 모드 열거형
enum DisplayMode {
    MODE_GH1_TEMP = 0,
    MODE_GH1_HUM,
    MODE_GH2_TEMP,
    MODE_GH2_HUM,
    MODE_GH3_TEMP,
    MODE_GH3_HUM,
    MODE_GH4_TEMP,
    MODE_GH4_HUM,
    MODE_AUTO_ROTATE,  // 자동 순환 모드
    MODE_COUNT
};

#endif // CONFIG_H
