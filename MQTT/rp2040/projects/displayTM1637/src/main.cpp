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

// 센서 데이터 저장 구조체
typedef struct {
    float gh1_temp;
    float gh1_hum;
    float gh2_temp;
    float gh2_hum;
    float gh3_temp;
    float gh3_hum;
    float gh4_temp;
    float gh4_hum;
    bool gh1_temp_valid;
    bool gh1_hum_valid;
    bool gh2_temp_valid;
    bool gh2_hum_valid;
    bool gh3_temp_valid;
    bool gh3_hum_valid;
    bool gh4_temp_valid;
    bool gh4_hum_valid;
} SensorData;

// 전역 변수
static SensorData sensor_data = {0};
static DisplayMode current_mode = MODE_GH1_TEMP;
static DisplayMode display_mode = MODE_AUTO_ROTATE;

/**
 * @brief MQTT 재연결 후 초기화 작업 수행
 */
bool mqtt_reinitialize_after_reconnect(MqttClient& mqtt) {
    printf("[MQTT] 재연결 후 초기화 시작...\n");
    
    // 상태 토픽에 online 메시지 발행 (retain)
    if (!mqtt_publish(mqtt, TOPIC_STATUS, "online", 0, 1)) {
        printf("[오류] 상태 메시지 발행 실패: %s\n", TOPIC_STATUS);
        return false;
    }
    
    // 센서 토픽 구독 (GH1~GH4)
    const char* topics[] = {
        TOPIC_GH1_TEMP, TOPIC_GH1_HUM,
        TOPIC_GH2_TEMP, TOPIC_GH2_HUM,
        TOPIC_GH3_TEMP, TOPIC_GH3_HUM,
        TOPIC_GH4_TEMP, TOPIC_GH4_HUM,
        TOPIC_DISPLAY_MODE
    };
    
    for (size_t i = 0; i < sizeof(topics) / sizeof(topics[0]); i++) {
        if (!mqtt_subscribe(mqtt, topics[i], 0)) {
            printf("[오류] 토픽 재구독 실패: %s\n", topics[i]);
            return false;
        }
        printf("[MQTT] 토픽 재구독 완료: %s\n", topics[i]);
    }
    
    printf("[MQTT] 재연결 후 초기화 완료\n");
    return true;
}

/**
 * @brief MQTT 메시지에서 float 값 파싱
 */
float parse_float_from_message(const char* message) {
    return atof(message);
}

/**
 * @brief MQTT 메시지 처리 (센서 데이터 업데이트)
 */
void process_mqtt_message(const char* topic, const char* message) {
    printf("[수신] %s: %s\n", topic, message);
    
    // 온도/습도 데이터 파싱
    if (strcmp(topic, TOPIC_GH1_TEMP) == 0) {
        sensor_data.gh1_temp = parse_float_from_message(message);
        sensor_data.gh1_temp_valid = true;
    }
    else if (strcmp(topic, TOPIC_GH1_HUM) == 0) {
        sensor_data.gh1_hum = parse_float_from_message(message);
        sensor_data.gh1_hum_valid = true;
    }
    else if (strcmp(topic, TOPIC_GH2_TEMP) == 0) {
        sensor_data.gh2_temp = parse_float_from_message(message);
        sensor_data.gh2_temp_valid = true;
    }
    else if (strcmp(topic, TOPIC_GH2_HUM) == 0) {
        sensor_data.gh2_hum = parse_float_from_message(message);
        sensor_data.gh2_hum_valid = true;
    }
    else if (strcmp(topic, TOPIC_GH3_TEMP) == 0) {
        sensor_data.gh3_temp = parse_float_from_message(message);
        sensor_data.gh3_temp_valid = true;
    }
    else if (strcmp(topic, TOPIC_GH3_HUM) == 0) {
        sensor_data.gh3_hum = parse_float_from_message(message);
        sensor_data.gh3_hum_valid = true;
    }
    else if (strcmp(topic, TOPIC_GH4_TEMP) == 0) {
        sensor_data.gh4_temp = parse_float_from_message(message);
        sensor_data.gh4_temp_valid = true;
    }
    else if (strcmp(topic, TOPIC_GH4_HUM) == 0) {
        sensor_data.gh4_hum = parse_float_from_message(message);
        sensor_data.gh4_hum_valid = true;
    }
    else if (strcmp(topic, TOPIC_DISPLAY_MODE) == 0) {
        int mode = atoi(message);
        if (mode >= 0 && mode < MODE_COUNT) {
            display_mode = (DisplayMode)mode;
            printf("[제어] 디스플레이 모드 변경: %d\n", mode);
        }
    }
}

/**
 * @brief 현재 모드에 따라 디스플레이 업데이트
 */
void update_display(TM1637Display& display, DisplayMode mode) {
    float value = 0;
    bool valid = false;
    
    switch (mode) {
        case MODE_GH1_TEMP:
            value = sensor_data.gh1_temp;
            valid = sensor_data.gh1_temp_valid;
            break;
        case MODE_GH1_HUM:
            value = sensor_data.gh1_hum;
            valid = sensor_data.gh1_hum_valid;
            break;
        case MODE_GH2_TEMP:
            value = sensor_data.gh2_temp;
            valid = sensor_data.gh2_temp_valid;
            break;
        case MODE_GH2_HUM:
            value = sensor_data.gh2_hum;
            valid = sensor_data.gh2_hum_valid;
            break;
        case MODE_GH3_TEMP:
            value = sensor_data.gh3_temp;
            valid = sensor_data.gh3_temp_valid;
            break;
        case MODE_GH3_HUM:
            value = sensor_data.gh3_hum;
            valid = sensor_data.gh3_hum_valid;
            break;
        case MODE_GH4_TEMP:
            value = sensor_data.gh4_temp;
            valid = sensor_data.gh4_temp_valid;
            break;
        case MODE_GH4_HUM:
            value = sensor_data.gh4_hum;
            valid = sensor_data.gh4_hum_valid;
            break;
        default:
            break;
    }
    
    if (valid) {
        if (mode % 2 == 0) {
            // 온도 표시
            tm1637_show_temperature(display, value);
        } else {
            // 습도 표시
            tm1637_show_humidity(display, value);
        }
    } else {
        // 데이터 없을 때 "----" 표시
        tm1637_clear(display);
    }
}

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
    printf("MQTT 센서 데이터 표시 시스템\n\n");
    
    // TM1637 디스플레이 초기화
    TM1637Display display = {
        .clk_pin = TM1637_CLK_PIN,
        .dio_pin = TM1637_DIO_PIN,
        .brightness = TM1637_BRIGHTNESS,
        .display_on = true
    };
    
    if (!tm1637_init(display)) {
        printf("[오류] TM1637 초기화 실패\n");
        return -1;
    }
    printf("[OK] TM1637 초기화 완료\n");
    
    // 초기 테스트 표시
    tm1637_show_number(display, 8888, true);
    sleep_ms(1000);
    tm1637_clear(display);
    
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
    uint32_t last_mode_rotation = 0;
    
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
        
        // 자동 회전 모드 (5초마다 모드 변경)
        if (display_mode == MODE_AUTO_ROTATE) {
            if (now - last_mode_rotation > 5000) {
                current_mode = (DisplayMode)((current_mode + 1) % (MODE_COUNT - 1));
                printf("[디스플레이] 모드 회전: %d\n", current_mode);
                last_mode_rotation = now;
            }
        } else {
            current_mode = display_mode;
        }
        
        // 디스플레이 업데이트 (1초마다)
        if (now - last_display_update > 1000) {
            update_display(display, current_mode);
            last_display_update = now;
        }
        
        sleep_ms(50);
    }
    
    return 0;
}
