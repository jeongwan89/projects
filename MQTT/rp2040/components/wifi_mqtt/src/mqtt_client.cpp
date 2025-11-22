#include "mqtt_client.h"
#include "uart_comm.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

bool mqtt_connect(MqttClient& client) {
    printf("[MQTT] 연결 시작: %s:%d\n", client.broker, client.port);
    
    char cmd[256];
    
    // 1. MQTT 사용자 설정
    snprintf(cmd, sizeof(cmd), "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
             client.client_id, client.username, client.password);
    uart_send_at_command(cmd);
    if (!uart_wait_response("OK", 2000)) {
        printf("[MQTT] 사용자 설정 실패\n");
        return false;
    }
    sleep_ms(300);
    
    // 2. MQTT 연결 설정 (LWT)
    snprintf(cmd, sizeof(cmd), "AT+MQTTCONNCFG=0,120,0,\"%s\",\"%s\",1,0",
             client.lwt_topic, client.lwt_message);
    uart_send_at_command(cmd);
    if (!uart_wait_response("OK", 2000)) {
        printf("[MQTT] 연결 설정 실패\n");
        return false;
    }
    sleep_ms(300);
    
    // 3. MQTT 브로커 연결
    snprintf(cmd, sizeof(cmd), "AT+MQTTCONN=0,\"%s\",%d,0", client.broker, client.port);
    uart_clear_rx_buffer();
    uart_send_at_command(cmd);
    
    if (!uart_wait_response("MQTTCONNECTED", 10000)) {
        printf("[MQTT] 브로커 연결 실패\n");
        return false;
    }
    
    client.connected = true;
    printf("[MQTT] 연결 성공\n");
    
    // 연결 성공 시 online 상태 발행
    mqtt_publish(client, client.lwt_topic, "online", 1, 1);
    
    return true;
}

bool mqtt_subscribe(MqttClient& client, const char* topic, int qos) {
    if (!client.connected) {
        printf("[MQTT] 연결되지 않음\n");
        return false;
    }
    
    printf("[MQTT] 구독: %s (QoS %d)\n", topic, qos);
    
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",%d", topic, qos);
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
    
    int msg_len = strlen(message);
    char cmd[256];
    
    // RX 버퍼 클리어
    uart_clear_rx_buffer();
    sleep_ms(100);
    
    // MQTTPUBRAW 명령 전송
    snprintf(cmd, sizeof(cmd), "AT+MQTTPUBRAW=0,\"%s\",%d,%d,%d",
             topic, msg_len, qos, retain);
    uart_send_at_command(cmd);
    
    // ">" 프롬프트 대기
    if (!uart_wait_response(">", 2000)) {
        printf("[MQTT] 발행 준비 실패\n");
        return false;
    }
    
    // 약간의 지연 후 메시지 전송
    sleep_ms(50);
    uart_clear_rx_buffer();
    
    // 메시지 전송 (개행 없이)
    uart_send_raw(message, msg_len);
    
    // 전송 완료 대기
    if (!uart_wait_response("OK", 3000)) {
        printf("[MQTT] 발행 실패\n");
        return false;
    }
    
    sleep_ms(100);
    return true;
}

bool mqtt_check_message(MqttClient& client, char* topic, char* message, int max_len) {
    (void)client;  // 현재는 사용하지 않지만 일관성을 위해 유지
    char buffer[256];
    int len = uart_read_mqtt_message(buffer, sizeof(buffer));
    
    if (len > 0) {
        // +MQTTSUBRECV:0,"topic",len,data 파싱
        char* p = strstr(buffer, "+MQTTSUBRECV");
        if (p) {
            // 간단한 파싱 (실제로는 더 정교해야 함)
            char* topic_start = strchr(p, '"');
            if (topic_start) {
                topic_start++;
                char* topic_end = strchr(topic_start, '"');
                if (topic_end) {
                    int topic_len = topic_end - topic_start;
                    if (topic_len < max_len) {
                        strncpy(topic, topic_start, topic_len);
                        topic[topic_len] = '\0';
                        
                        // 메시지 데이터 추출 (간략화)
                        char* msg_start = strchr(topic_end + 1, ',');
                        if (msg_start) {
                            msg_start = strchr(msg_start + 1, ',');
                            if (msg_start) {
                                strncpy(message, msg_start + 1, max_len - 1);
                                message[max_len - 1] = '\0';
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool mqtt_is_connected(MqttClient& client) {
    return client.connected;
}

void mqtt_disconnect(MqttClient& client) {
    if (client.connected) {
        uart_send_at_command("AT+MQTTCLEAN=0");
        uart_wait_response("OK", 2000);
        client.connected = false;
        printf("[MQTT] 연결 해제\n");
    }
}
