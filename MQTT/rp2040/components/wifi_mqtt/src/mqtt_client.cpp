
#include "esp01.h"
#include "uart_comm.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "mqtt_client.h"

// UART 설정을 위한 전역 포인터 (main에서 할당)
static const uart_config_t* uart_cfg_ptr = NULL;

void mqtt_client_set_uart_config(const uart_config_t* cfg) {
    uart_cfg_ptr = cfg;
}

static bool mqtt_connected = false;
static uint32_t last_publish_success = 0;

bool mqtt_connect(const mqtt_client_config_t& cfg) {
    printf("=== MQTT 연결 ===\n");
    char cmd[256];
    // MQTT 사용자 설정
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
             cfg.client_id, cfg.username, cfg.password);
    send_at_command(*uart_cfg_ptr, cmd);
    if (!wait_for_response("OK", 2000)) {
        printf("MQTT 사용자 설정 실패\n");
        return false;
    }
    sleep_ms(500);
    // MQTT 연결 설정 (LWT 포함)
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTCONNCFG=0,120,0,\"%s\",\"%s\",1,0",
             cfg.lwt_topic, cfg.lwt_message);
    send_at_command(*uart_cfg_ptr, cmd);
    if (!wait_for_response("OK", 2000)) {
        printf("MQTT 연결 설정 실패\n");
        return false;
    }
    sleep_ms(300);
    // MQTT 브로커 연결
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTCONN=0,\"%s\",%d,0",
             cfg.broker, cfg.port);
    send_at_command(*uart_cfg_ptr, cmd);
    // MQTT 연결 완료 대기 (MQTTCONNECTED 메시지 직접 기다림)
    printf("MQTT 연결 완료 대기 중...\n");
    if (!wait_for_response("MQTTCONNECTED", 10000)) {
        printf("MQTT 브로커 연결 타임아웃\n");
        return false;
    }
    sleep_ms(1000);
    printf("MQTT 연결 성공\n");
    mqtt_connected = true;
    // 연결 성공 시 online 상태 발행
    char status_cmd[128];
    snprintf(status_cmd, sizeof(status_cmd), "AT+MQTTPUB=0,\"%s\",\"online\",1,1", MQTT_TOPIC_STATUS);
    send_at_command(*uart_cfg_ptr, status_cmd);
    wait_for_response("OK", 2000);
    return true;
}

bool mqtt_subscribe(const mqtt_client_config_t& cfg, const char* topic) {
    (void)cfg;
    if (!mqtt_connected) {
        printf("MQTT 연결되지 않음 - 구독 불가\n");
        return false;
    }
    
    printf("[EVENT] === MQTT 구독: %s ===\n", topic);
    
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",1", topic);
    send_at_command(*uart_cfg_ptr, cmd);
    
    if (!wait_for_response("OK", 3000)) {
        printf("구독 실패\n");
        return false;
    }

    // 구독 성공 시 UART 버퍼 클리어

    sleep_ms(300);  // 구독 처리 대기
    printf("구독 성공\n");
    return true;
}

bool mqtt_publish(const mqtt_client_config_t& cfg, const char* topic, const char* message) {
    (void)cfg;
    if (!mqtt_connected) {
        printf("[EVENT] MQTT 연결되지 않음 - 발행 불가\n");
        return false;
    }
    
    int msg_len = strlen(message);
    char cmd[256];
    
    // MQTTPUBRAW 사용 (길이 기반 전송 - 특수문자 안전)
    snprintf(cmd, sizeof(cmd), 
             "AT+MQTTPUBRAW=0,\"%s\",%d,1,0",
             topic, msg_len);
    printf("[TX] %s\n", cmd);
    send_at_command(*uart_cfg_ptr, cmd);
    
    // ">" 프롬프트 대기
    if (!wait_for_response(">", 2000)) {
        printf("[EVENT] 발행 준비 실패 (> 프롬프트 없음) - MQTT 연결 끊김 감지\n");
        mqtt_connected = false;  // 연결 끊김 표시
        return false;
    }
    
    // 실제 메시지 전송 (개행 없이)
    uart_puts(uart_cfg_ptr->uart_id, message);
    printf("[TX] Send Data: %s\n", message);
    
    if (!wait_for_response("OK", 3000)) {
        printf("[EVENT] 발행 실패 - MQTT 연결 끊김 가능성\n");
        mqtt_connected = false;  // 연결 끊김 표시
        return false;
    }
    
    sleep_ms(200);  // 발행 완료 대기
    // 발행 성공 시 UART 버퍼 클리어
    last_publish_success = to_ms_since_boot(get_absolute_time());
    return true;
}

void check_mqtt_messages(const mqtt_client_config_t& cfg) {
    (void)cfg;
    // MQTT 전용 버퍼로 데이터 읽기
    uart_read_mqtt_messages();
    
    const char* buffer = get_mqtt_buffer();
    int buffer_len = strlen(buffer);
    
    // 버퍼에 데이터가 없으면 리턴
    if (buffer_len == 0) {
        return;
    }
    
    // +MQTTSUBRECV가 시작되었지만 완전하지 않으면 추가 대기
    if (strstr(buffer, "+MQTTSUBRECV") != NULL) {
        bool is_complete = (strchr(buffer, '\n') != NULL);
        
        // 인터럽트가 이미 수집했으므로 한 번만 더 읽기
        if (!is_complete) {
            uart_read_mqtt_messages();
            buffer = get_mqtt_buffer();
            is_complete = (strchr(buffer, '\n') != NULL);
        }
        
        // 여전히 불완전하면 강제 처리 (길이가 40자 이상이면)
        if (!is_complete && strlen(buffer) < 40) {
            return; // 너무 짧으면 다음 기회에
        }
        
        printf("[RX] Recv: %s", buffer);
        
        // 제어 명령 처리
        // 형식: +MQTTSUBRECV:0,"test/rp2040/control",2,ON
        if (strstr(buffer, ",ON") != NULL || strstr(buffer, " ON") != NULL) {
            printf("[EVENT] ON 명령 수신 - LED/장치 켜기\n");
            // TODO: GPIO 제어 코드
            // 예: gpio_put(LED_PIN, 1);
        } else if (strstr(buffer, ",OFF") != NULL || strstr(buffer, " OFF") != NULL) {
            printf("[EVENT] OFF 명령 수신 - LED/장치 끄기\n");
            // TODO: GPIO 제어 코드
            // 예: gpio_put(LED_PIN, 0);
        } else if (strstr(buffer, "STATUS") != NULL) {
            printf("[EVENT] STATUS 명령 수신 - 상태 보고\n");
            // TODO: 현재 상태를 MQTT로 발행
            // mqtt_publish(MQTT_TOPIC_ALIVE, "status:ok");
        }
        
        clear_mqtt_buffer();
    }
}

bool mqtt_is_connected(const mqtt_client_config_t& cfg) {
    (void)cfg;
    return mqtt_connected;
}

void mqtt_disconnect(const mqtt_client_config_t& cfg) {
    (void)cfg;
    printf("=== MQTT 연결 해제 ===\n");
    send_at_command(*uart_cfg_ptr, "AT+MQTTCLEAN=0");
    wait_for_response("OK", 2000);  // 타임아웃 무시
    mqtt_connected = false;
    last_publish_success = 0;
    printf("MQTT 연결 해제 완료\n");
}

// MQTT 재연결: 연결 해제 → ESP-01 리셋/재초기화 → WiFi 재연결 → MQTT 재연결
bool mqtt_reconnect(const mqtt_client_config_t& mqtt_cfg, const esp01_config_t& esp_cfg) {
    // 1. MQTT 연결 해제
    mqtt_disconnect(mqtt_cfg);
    sleep_ms(500);

    // 2. ESP-01 하드웨어 리셋
    esp01_hardware_reset(esp_cfg);
    sleep_ms(500);

    // 3. ESP-01 재초기화
    if (!esp01_init(esp_cfg)) {
        printf("[RECONNECT] ESP-01 초기화 실패\n");
        return false;
    }

    // 4. WiFi 재연결
    if (!wifi_connect(esp_cfg)) {
        printf("[RECONNECT] WiFi 재연결 실패\n");
        return false;
    }

    // 5. MQTT 재연결
    if (!mqtt_connect(mqtt_cfg)) {
        printf("[RECONNECT] MQTT 재연결 실패\n");
        return false;
    }

    printf("[RECONNECT] MQTT 재연결 성공\n");
    return true;
}


