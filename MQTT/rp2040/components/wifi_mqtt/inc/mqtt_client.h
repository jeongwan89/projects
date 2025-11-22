#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// MQTT 클라이언트 설정 구조체
typedef struct {
    const char* broker;       // MQTT 브로커 주소
    int port;                 // 브로커 포트
    const char* client_id;    // 클라이언트 ID
    const char* username;     // 사용자명
    const char* password;     // 비밀번호
    const char* lwt_topic;    // Last Will Topic
    const char* lwt_message;  // Last Will Message
    bool connected;           // 연결 상태
} MqttClient;

// MQTT 브로커 연결
bool mqtt_connect(MqttClient& client);

// MQTT 토픽 구독
bool mqtt_subscribe(MqttClient& client, const char* topic, int qos);

// MQTT 메시지 발행
bool mqtt_publish(MqttClient& client, const char* topic, const char* message, int qos, int retain);

// MQTT 메시지 체크 (수신 확인)
bool mqtt_check_message(MqttClient& client, char* topic, int topic_max_len, char* message, int message_max_len);

// MQTT 연결 상태
bool mqtt_is_connected(MqttClient& client);

// MQTT 연결 해제
void mqtt_disconnect(MqttClient& client);

#ifdef __cplusplus
}
#endif

#endif // MQTT_CLIENT_H
