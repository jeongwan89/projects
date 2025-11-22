#ifndef CONFIG_H
#define CONFIG_H

// UART 설정
#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5

// ESP-01 제어 핀
#define ESP01_RST_PIN 3

// MQTT 서버 설정
#define MQTT_BROKER "192.168.0.24"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "rp2040_client"
#define MQTT_USERNAME "farmmain"
#define MQTT_PASSWORD "eerrtt"
#define MQTT_TOPIC_ALIVE "test/rp2040/alive"
#define MQTT_TOPIC_SENSOR "test/rp2040/sensor"
#define MQTT_TOPIC_CONTROL "test/rp2040/control"
#define MQTT_TOPIC_STATUS "test/rp2040/status"
#define MQTT_LWT_MESSAGE "offline"

// WiFi 설정
#define WIFI_SSID "FarmMain"
#define WIFI_PASSWORD "wweerrtt"

// 버퍼 설정
#define RX_BUFFER_SIZE 1024

// 타이밍 설정
#define SENSOR_SEND_INTERVAL_MS 10000  // 10초
#define ALIVE_SEND_INTERVAL_MS 60000   // 60초
#define ESP01_RESET_DELAY_MS 5000      // ESP-01 리셋 후 대기 시간

#endif // CONFIG_H
