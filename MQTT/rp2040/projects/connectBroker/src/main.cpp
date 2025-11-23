#include <stdio.h>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "uart_comm.h"
#include "esp01.h"
#include "mqtt_client.h"
#include "serial_bridge.h"
#include "config.h"

int main(void) {
    // 표준 입출력 초기화
    stdio_init_all();
    
    // USB CDC 연결 대기 (최대 5초)
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while (!stdio_usb_connected() && (to_ms_since_boot(get_absolute_time()) - start < 5000)) {
        sleep_ms(100);
    }
    sleep_ms(500);
    
    printf("\n\n=== RP2040 MQTT 클라이언트 ===\n");
    printf("ESP-01 WiFi & MQTT 브로커 연결\n\n");
    
    // ESP-01 모듈 설정
    Esp01Module esp01 = {
        .uart = ESP01_UART,
        .uart_tx_pin = ESP01_UART_TX_PIN,
        .uart_rx_pin = ESP01_UART_RX_PIN,
        .uart_baudrate = ESP01_UART_BAUDRATE,
        .rst_pin = ESP01_RST_PIN,
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD
    };
    
    // ESP-01 모듈 초기화 (UART + 하드웨어 리셋)
    esp01_module_init(esp01);
    
    // ESP-01 AT 명령 초기화
    if (!esp01_at_init(esp01)) {
        printf("[오류] ESP-01 AT 초기화 실패\n");
        printf("시리얼 브리지 모드로 전환합니다...\n");
        serial_bridge_mode(uart1);
        return -1;
    }
    
    // WiFi 연결
    if (!esp01_connect_wifi(esp01)) {
        printf("[오류] WiFi 연결 실패\n");
        printf("시리얼 브리지 모드로 전환합니다...\n");
        serial_bridge_mode(uart1);
        return -1;
    }
    
    sleep_ms(2000);
    
    // MQTT 클라이언트 설정
    MqttClient mqtt = {
        .broker = MQTT_BROKER,
        .port = MQTT_PORT,
        .client_id = MQTT_CLIENT_ID,
        .username = MQTT_USERNAME,
        .password = MQTT_PASSWORD,
        .lwt_topic = LWT_TOPIC,
        .lwt_message = LWT_MESSAGE,
        .connected = false
    };
    
    // MQTT 브로커 연결
    if (!mqtt_connect(mqtt)) {
        printf("[오류] MQTT 연결 실패\n");
        return -1;
    }
    
    // 제어 토픽 구독
    mqtt_subscribe(mqtt, TOPIC_CONTROL, 0);
    
    // Alive 메시지 발행
    mqtt_publish(mqtt, TOPIC_STATUS, "online", 0, 1);
    
    printf("\n=== 메인 루프 시작 ===\n");
    
    uint32_t last_sensor_time = 0;
    uint32_t last_alive_time = 0;
    uint32_t last_connection_check = 0;
    
    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        // 연결 상태 확인 (30초마다)
        if (now - last_connection_check > 30000) {
            if (!mqtt_is_connected(mqtt)) {
                printf("[경고] MQTT 연결 끊김 감지\n");
                
                // WiFi 연결 확인
                if (!esp01_is_connected(esp01)) {
                    printf("[경고] WiFi 연결 끊김 감지\n");
                    if (esp01_reconnect_wifi(esp01)) {
                        // WiFi 재연결 성공 후 MQTT 재연결
                        mqtt_reconnect(mqtt);
                    }
                } else {
                    // WiFi는 연결됨, MQTT만 재연결
                    mqtt_reconnect(mqtt);
                }
            }
            last_connection_check = now;
        }
        
        // 센서 데이터 발행 (10초마다)
        if (now - last_sensor_time > 10000) {
            char data[64];
            float temp = 25.5;  // TODO: 실제 센서 값
            float humi = 60.0;  // TODO: 실제 센서 값
            snprintf(data, sizeof(data), "{\"temp\":%.1f,\"humi\":%.1f}", temp, humi);
            
            printf("[발행] %s: %s\n", TOPIC_SENSOR, data);
            if (!mqtt_publish(mqtt, TOPIC_SENSOR, data, 0, 0)) {
                printf("[경고] 센서 데이터 발행 실패 - 연결 확인 중...\n");
                // 즉시 연결 재확인
                last_connection_check = 0;
            }
            last_sensor_time = now;
        }
        
        // Alive 메시지 (60초마다)
        if (now - last_alive_time > 60000) {
            printf("[발행] %s: alive\n", TOPIC_STATUS);
            if (!mqtt_publish(mqtt, TOPIC_STATUS, "alive", 0, 0)) {
                printf("[경고] Alive 메시지 발행 실패 - 연결 확인 중...\n");
                // 즉시 연결 재확인
                last_connection_check = 0;
            }
            last_alive_time = now;
        }
        
        // MQTT 메시지 수신 확인
        char topic[64], message[128];
        if (mqtt_check_message(mqtt, topic, sizeof(topic), message, sizeof(message))) {
            printf("[수신] %s: %s\n", topic, message);
            
            // 제어 명령 처리
            if (strstr(message, "ON")) {
                printf("[제어] 장치 ON\n");
                // TODO: 액츄에이터 제어
            } else if (strstr(message, "OFF")) {
                printf("[제어] 장치 OFF\n");
                // TODO: 액츄에이터 제어
            }
        }
        
        sleep_ms(50);
    }
    
    return 0;
}
