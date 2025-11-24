#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "uart_comm.h"
#include "esp01.h"
#include "mqtt_client.h"
#include "serial_bridge.h"
#include "tm1637.h"
#include "config.h"
#include "main.h"

// 전역 변수 정의
TM1637Display displays[NUM_DISPLAYS];
DisplayData display_data[NUM_DISPLAYS] = {0};

int main(void) {
    // 표준 입출력 초기화
    stdio_init_all();
    
    // USB CDC 연결 대기 (최대 5초)
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while (!stdio_usb_connected() && (to_ms_since_boot(get_absolute_time()) - start < 5000)) {
        sleep_ms(100);
    }
    sleep_ms(500);
    
    printf("\n\n=== RP2040 TM1637 디스플레이 ===\n");
    printf("8개 디스플레이 - 온실 4개 (온도/습도)\n\n");
    
    // 8개 TM1637 디스플레이 초기화
    const uint8_t display_pins[NUM_DISPLAYS][2] = {
        {TM1637_GH1_TEMP_CLK, TM1637_GH1_TEMP_DIO}, // GH1 온도
        {TM1637_GH1_HUM_CLK,  TM1637_GH1_HUM_DIO},  // GH1 습도
        {TM1637_GH2_TEMP_CLK, TM1637_GH2_TEMP_DIO}, // GH2 온도
        {TM1637_GH2_HUM_CLK,  TM1637_GH2_HUM_DIO},  // GH2 습도
        {TM1637_GH3_TEMP_CLK, TM1637_GH3_TEMP_DIO}, // GH3 온도
        {TM1637_GH3_HUM_CLK,  TM1637_GH3_HUM_DIO},  // GH3 습도
        {TM1637_GH4_TEMP_CLK, TM1637_GH4_TEMP_DIO}, // GH4 온도
        {TM1637_GH4_HUM_CLK,  TM1637_GH4_HUM_DIO}   // GH4 습도
    };
    
    const char* display_names[NUM_DISPLAYS] = {
        "GH1 온도", "GH1 습도",
        "GH2 온도", "GH2 습도",
        "GH3 온도", "GH3 습도",
        "GH4 온도", "GH4 습도"
    };
    
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        displays[i].clk_pin = display_pins[i][0];
        displays[i].dio_pin = display_pins[i][1];
        displays[i].brightness = TM1637_BRIGHTNESS;
        displays[i].display_on = true;
        
        if (!tm1637_init(displays[i])) {
            printf("[오류] %s 디스플레이 초기화 실패 (CLK=%d, DIO=%d)\n", 
                   display_names[i], display_pins[i][0], display_pins[i][1]);
            return -1;
        }
        printf("[OK] %s 디스플레이 초기화 완료 (CLK=%d, DIO=%d)\n", 
               display_names[i], display_pins[i][0], display_pins[i][1]);
    }
    
    // 초기 테스트 표시 (모든 디스플레이에 8888)
    printf("\n디스플레이 테스트 중...\n");
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        tm1637_show_number(displays[i], 8888, true);
    }
    sleep_ms(1000);
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        tm1637_clear(displays[i]);
    }
    printf("디스플레이 테스트 완료\n\n");
    
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
    
    // ESP-01 모듈 초기화
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
        .connected = false,
        .last_activity = 0
    };
    
    // MQTT 브로커 연결
    if (!mqtt_connect(mqtt)) {
        printf("[오류] MQTT 연결 실패\n");
        return -1;
    }
    
    // MQTT 초기화 (구독 + 상태 발행)
    if (!mqtt_reinitialize_after_reconnect(mqtt)) {
        printf("[경고] MQTT 초기화 실패\n");
    }
    
    printf("\n=== 메인 루프 시작 ===\n");
    
    uint32_t last_connection_check = 0;
    uint32_t last_display_update = 0;
    
    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        // 연결 상태 확인 (30초마다)
        if (now - last_connection_check > 30000) {
            mqtt_keepalive(mqtt);
            
            if (!mqtt_is_connected(mqtt)) {
                printf("[경고] MQTT 연결 끊김 감지\n");
                
                if (!esp01_is_connected(esp01)) {
                    printf("[경고] WiFi 연결 끊김 감지\n");
                    if (esp01_reconnect_wifi(esp01)) {
                        if (mqtt_reconnect(mqtt)) {
                            mqtt_reinitialize_after_reconnect(mqtt);
                        }
                    }
                } else {
                    if (mqtt_reconnect(mqtt)) {
                        mqtt_reinitialize_after_reconnect(mqtt);
                    }
                }
            }
            last_connection_check = now;
        }
        
        // MQTT 메시지 수신 확인
        char topic[64], message[128];
        if (mqtt_check_message(mqtt, topic, sizeof(topic), message, sizeof(message))) {
            process_mqtt_message(topic, message);
        }
        
        // 모든 디스플레이 업데이트 (1초마다)
        if (now - last_display_update > 1000) {
            update_all_displays();
            last_display_update = now;
        }
        
        sleep_ms(50);
    }
    
    return 0;
}
