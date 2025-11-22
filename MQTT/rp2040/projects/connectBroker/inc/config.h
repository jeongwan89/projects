#ifndef CONFIG_H
#define CONFIG_H

#include "mqtt_client.h"
#include "esp01.h"
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

// MQTT 브로커 설정
#define MQTT_BROKER     "192.168.0.24"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "rp2040_client_prototype_001"
#define MQTT_USERNAME   "farmmain"
#define MQTT_PASSWORD   "eerrtt"

// MQTT 토픽
#define TOPIC_STATUS    "test/rp2040/status"
#define TOPIC_SENSOR    "test/rp2040/sensor"
#define TOPIC_CONTROL   "test/rp2040/control"

// LWT (Last Will Testament)
#define LWT_TOPIC       TOPIC_STATUS
#define LWT_MESSAGE     "offline"

#endif // CONFIG_H
