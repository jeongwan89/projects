#include "mqtt_client.h"
#include "uart_comm.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "pico/stdlib.h"

// AT 명령어 최대 길이 제한
#define MAX_AT_COMMAND_LEN 512
#define MAX_TOPIC_LEN 128
#define MAX_CLIENT_ID_LEN 64
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 64
#define MQTT_RX_BUFFER_SIZE 1024  // UART RX_BUFFER_SIZE와 일치
#define MAX_BROKER_LEN 128

bool mqtt_connect(MqttClient& client) {
    // NULL 포인터 검증
    if (!client.broker || !client.client_id || !client.username || 
        !client.password || !client.lwt_topic || !client.lwt_message) {
        printf("[MQTT] NULL 포인터 오류\n");
        return false;
    }
    
    // 길이 검증
    if (strlen(client.client_id) >= MAX_CLIENT_ID_LEN ||
        strlen(client.username) >= MAX_USERNAME_LEN ||
        strlen(client.password) >= MAX_PASSWORD_LEN) {
        printf("[MQTT] 사용자 정보 길이 초과\n");
        return false;
    }
    
    // Broker 주소 길이 검증
    if (strlen(client.broker) == 0 || strlen(client.broker) >= MAX_BROKER_LEN) {
        printf("[MQTT] 브로커 주소 유효하지 않음\n");
        return false;
    }
    
    // Port 범위 검증 (1-65535)
    if (client.port < 1 || client.port > 65535) {
        printf("[MQTT] 유효하지 않은 포트: %d\n", client.port);
        return false;
    }
    
    printf("[MQTT] 연결 시작: %.*s:%d\n", MAX_BROKER_LEN, client.broker, client.port);
    
    char cmd[MAX_AT_COMMAND_LEN];
    int cmd_len;
    
    // 1. MQTT 사용자 설정
    cmd_len = snprintf(cmd, sizeof(cmd), "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
                       client.client_id, client.username, client.password);
    if (cmd_len >= (int)sizeof(cmd)) {
        printf("[MQTT] 사용자 설정 명령어 버퍼 오버플로우\n");
        return false;
    }
    
    uart_send_at_command(cmd);
    if (!uart_wait_response("OK", 2000)) {
        printf("[MQTT] 사용자 설정 실패\n");
        client.connected = false;
        return false;
    }
    sleep_ms(300);
    
    // 2. MQTT 연결 설정 (LWT)
    if (strlen(client.lwt_topic) >= MAX_TOPIC_LEN) {
        printf("[MQTT] LWT 토픽 길이 초과\n");
        return false;
    }
    
    cmd_len = snprintf(cmd, sizeof(cmd), "AT+MQTTCONNCFG=0,120,0,\"%s\",\"%s\",1,0",
                       client.lwt_topic, client.lwt_message);
    if (cmd_len >= (int)sizeof(cmd)) {
        printf("[MQTT] LWT 설정 명령어 버퍼 오버플로우\n");
        return false;
    }
    
    uart_send_at_command(cmd);
    if (!uart_wait_response("OK", 2000)) {
        printf("[MQTT] 연결 설정 실패\n");
        client.connected = false;
        return false;
    }
    sleep_ms(300);
    
    // 3. MQTT 브로커 연결
    cmd_len = snprintf(cmd, sizeof(cmd), "AT+MQTTCONN=0,\"%s\",%d,0", client.broker, client.port);
    if (cmd_len >= (int)sizeof(cmd)) {
        printf("[MQTT] 브로커 연결 명령어 버퍼 오버플로우\n");
        return false;
    }
    
    uart_clear_rx_buffer();
    uart_send_at_command(cmd);
    
    if (!uart_wait_response("MQTTCONNECTED", 10000)) {
        printf("[MQTT] 브로커 연결 실패\n");
        client.connected = false;
        return false;
    }
    
    client.connected = true;
    printf("[MQTT] 연결 성공\n");
    
    // 연결 성공 시 online 상태 발행
    if (!mqtt_publish(client, client.lwt_topic, "online", 1, 1)) {
        printf("[MQTT] 경고: online 상태 발행 실패\n");
    }
    
    return true;
}

bool mqtt_subscribe(MqttClient& client, const char* topic, int qos) {
    if (!client.connected) {
        printf("[MQTT] 연결되지 않음\n");
        return false;
    }
    
    // NULL 포인터 검증
    if (!topic) {
        printf("[MQTT] NULL 토픽\n");
        return false;
    }
    
    // QoS 검증 (MQTT 표준: 0, 1, 2만 유효)
    if (qos < 0 || qos > 2) {
        printf("[MQTT] 유효하지 않은 QoS: %d\n", qos);
        return false;
    }
    
    // 토픽 길이 검증
    if (strlen(topic) >= MAX_TOPIC_LEN) {
        printf("[MQTT] 토픽 길이 초과\n");
        return false;
    }
    
    printf("[MQTT] 구독: %.*s (QoS %d)\n", MAX_TOPIC_LEN, topic, qos);
    
    char cmd[MAX_AT_COMMAND_LEN];
    int cmd_len = snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",%d", topic, qos);
    if (cmd_len >= (int)sizeof(cmd)) {
        printf("[MQTT] 구독 명령어 버퍼 오버플로우\n");
        return false;
    }
    
    uart_send_at_command(cmd);
    
    if (!uart_wait_response("OK", 3000)) {
        printf("[MQTT] 구독 실패\n");
        return false;
    }
    
    sleep_ms(300);
    return true;
}

bool mqtt_publish(MqttClient& client, const char* topic, const char* message, int qos, int retain) {
    if (!client.connected) {
        printf("[MQTT] 연결되지 않음\n");
        return false;
    }
    
    // NULL 포인터 검증
    if (!topic || !message) {
        printf("[MQTT] NULL 토픽 또는 메시지\n");
        return false;
    }
    
    // QoS 검증
    if (qos < 0 || qos > 2) {
        printf("[MQTT] 유효하지 않은 QoS: %d\n", qos);
        return false;
    }
    
    // retain 검증
    if (retain < 0 || retain > 1) {
        printf("[MQTT] 유효하지 않은 retain: %d\n", retain);
        return false;
    }
    
    // 토픽 길이 검증
    if (strlen(topic) >= MAX_TOPIC_LEN) {
        printf("[MQTT] 토픽 길이 초과\n");
        return false;
    }
    
    int msg_len = strlen(message);
    
    // 빈 메시지 경고 (허용하지만 의도하지 않은 동작일 수 있음)
    if (msg_len == 0) {
        printf("[MQTT] 경고: 빈 메시지 발행\n");
    }
    
    char cmd[MAX_AT_COMMAND_LEN];
    
    // RX 버퍼 클리어
    uart_clear_rx_buffer();
    sleep_ms(100);
    
    // MQTTPUBRAW 명령 전송
    int cmd_len = snprintf(cmd, sizeof(cmd), "AT+MQTTPUBRAW=0,\"%s\",%d,%d,%d",
                           topic, msg_len, qos, retain);
    if (cmd_len >= (int)sizeof(cmd)) {
        printf("[MQTT] 발행 명령어 버퍼 오버플로우\n");
        return false;
    }
    
    uart_send_at_command(cmd);
    
    // ">" 프롬프트 대기
    if (!uart_wait_response(">", 2000)) {
        printf("[MQTT] 발행 준비 실패 (연결 끊김 가능성)\n");
        client.connected = false;  // 연결 상태 플래그 업데이트
        return false;
    }
    
    // 약간의 지연 후 메시지 전송 (race condition 방지: 버퍼 클리어 제거)
    sleep_ms(50);
    
    // 메시지 전송 (개행 없이)
    uart_send_raw(message, msg_len);
    
    // 전송 완료 대기
    if (!uart_wait_response("OK", 3000)) {
        printf("[MQTT] 발행 실패 (연결 끊김 가능성)\n");
        client.connected = false;  // 연결 상태 플래그 업데이트
        return false;
    }
    
    sleep_ms(100);
    return true;
}

bool mqtt_check_message(MqttClient& client, char* topic, int topic_max_len, char* message, int message_max_len) {
    if (!client.connected) {
        return false;  // MQTT 연결되지 않았으면 메시지 체크 안 함
    }
    
    // NULL 포인터 검증
    if (!topic || !message) {
        printf("[MQTT] NULL 버퍼\n");
        return false;
    }
    
    char buffer[MQTT_RX_BUFFER_SIZE];  // 상수화된 버퍼 크기
    int len = uart_read_mqtt_message(buffer, sizeof(buffer));
    
    if (len <= 0) {
        return false;
    }
    
    // +MQTTSUBRECV:0,"topic",data_len,data 파싱
    char* p = strstr(buffer, "+MQTTSUBRECV:");
    if (!p) {
        printf("[MQTT] 메시지 포맷 오류: MQTTSUBRECV 없음\n");
        return false;
    }
    
    // 링크 ID 건너뛰기 (0)
    p = strchr(p, ',');
    if (!p) {
        printf("[MQTT] 메시지 포맷 오류: 링크 ID 없음\n");
        return false;
    }
    p++;  // ',' 다음으로
    
    // 토픽 추출: "topic"
    char* topic_start = strchr(p, '"');
    if (!topic_start) {
        printf("[MQTT] 메시지 포맷 오류: 토픽 시작 없음\n");
        return false;
    }
    topic_start++;  // '"' 다음으로
    
    char* topic_end = strchr(topic_start, '"');
    if (!topic_end) {
        printf("[MQTT] 메시지 포맷 오류: 토픽 끝 없음\n");
        return false;
    }
    
    int topic_len = topic_end - topic_start;
    if (topic_len >= topic_max_len) {
        printf("[MQTT] 토픽 버퍼 오버플로우: %d >= %d\n", topic_len, topic_max_len);
        return false;
    }
    
    strncpy(topic, topic_start, topic_len);
    topic[topic_len] = '\0';
    
    // 데이터 길이 파싱: ,len,
    p = strchr(topic_end + 1, ',');
    if (!p) {
        printf("[MQTT] 메시지 포맷 오류: 데이터 길이 구분자 없음\n");
        return false;
    }
    p++;  // ',' 다음으로
    
    // 정수 오버플로우 방지
    int data_len = 0;
    int digit_count = 0;
    char* buffer_end = buffer + len;  // 버퍼 끝 포인터
    
    while (*p >= '0' && *p <= '9' && p < buffer_end) {
        // INT_MAX 근처에서 오버플로우 방지
        if (data_len > (INT_MAX - 10) / 10) {
            printf("[MQTT] 데이터 길이 정수 오버플로우\n");
            return false;
        }
        
        data_len = data_len * 10 + (*p - '0');
        p++;
        digit_count++;
        
        // 비정상적으로 긴 숫자 문자열 방지 (최대 10자리)
        if (digit_count > 10) {
            printf("[MQTT] 데이터 길이 필드 너무 김\n");
            return false;
        }
    }
    
    // 버퍼 끝을 넘어선 경우
    if (p >= buffer_end) {
        printf("[MQTT] 파싱 중 버퍼 오버런\n");
        return false;
    }
    
    if (*p != ',') {
        printf("[MQTT] 메시지 포맷 오류: 데이터 시작 구분자 없음\n");
        return false;
    }
    p++;  // ',' 다음으로 (데이터 시작)
    
    // 메시지 데이터 복사
    if (data_len >= message_max_len) {
        printf("[MQTT] 메시지 버퍼 오버플로우: %d >= %d\n", data_len, message_max_len);
        // 잘라서라도 복사
        data_len = message_max_len - 1;
    }
    
    // 실제 남은 데이터 길이 확인
    int remaining = buffer_end - p;
    if (remaining < data_len) {
        printf("[MQTT] 경고: 예상 길이(%d)보다 실제 데이터 적음(%d)\n", data_len, remaining);
        data_len = remaining;
    }
    
    // 음수 길이 방지
    if (data_len < 0) {
        printf("[MQTT] 유효하지 않은 데이터 길이: %d\n", data_len);
        return false;
    }
    
    if (data_len > 0) {
        memcpy(message, p, data_len);
        message[data_len] = '\0';
    } else {
        message[0] = '\0';
    }
    
    printf("[MQTT] 파싱 성공 - 토픽: %s, 길이: %d\n", topic, data_len);
    return true;
}

bool mqtt_is_connected(MqttClient& client) {
    return client.connected;
}

bool mqtt_check_connection(MqttClient& client) {
    if (!client.connected) {
        return false;
    }
    
    // AT+MQTTCONN? 명령으로 실제 연결 상태 확인
    uart_clear_rx_buffer();
    sleep_ms(50);
    uart_send_at_command("AT+MQTTCONN?");
    
    // +MQTTCONN:0,0,"","",1883,0 응답 대기 (0=연결됨, 1=연결끊김)
    if (uart_wait_response("+MQTTCONN:", 2000)) {
        const char* resp = uart_get_rx_buffer();
        if (resp && strstr(resp, ",0,")) {
            // 첫 번째 파라미터가 0이면 연결됨
            return true;
        }
    }
    
    // 연결 상태 플래그 업데이트
    client.connected = false;
    return false;
}

bool mqtt_reconnect(MqttClient& client) {
    printf("[MQTT] 재연결 시도...\n");
    
    // 기존 연결 정리
    if (client.connected) {
        mqtt_disconnect(client);
        sleep_ms(1000);
    }
    
    // 재연결 시도 (3회)
    for (int i = 0; i < 3; i++) {
        if (mqtt_connect(client)) {
            printf("[MQTT] 재연결 성공\n");
            return true;
        }
        printf("[MQTT] 재연결 실패 (시도 %d/3)\n", i + 1);
        sleep_ms(2000);
    }
    
    printf("[MQTT] 재연결 최종 실패\n");
    return false;
}

void mqtt_disconnect(MqttClient& client) {
    if (client.connected) {
        uart_send_at_command("AT+MQTTCLEAN=0");
        if (!uart_wait_response("OK", 2000)) {
            printf("[MQTT] 연결 해제 실패 (타임아웃)\n");
        }
        client.connected = false;
        printf("[MQTT] 연결 해제\n");
    }
}
