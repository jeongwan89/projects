#include "uart_comm.h"
void mqtt_client_set_uart_config(const uart_config_t* cfg);
#define MQTT_TOPIC_ALIVE   "test/rp2040/alive"
#define MQTT_TOPIC_SENSOR  "test/rp2040/sensor"
#define MQTT_TOPIC_CONTROL "test/rp2040/control"
#define MQTT_TOPIC_STATUS  "test/rp2040/status"
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>
#include "esp01.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char* broker;
	int port;
	const char* client_id;
	const char* username;
	const char* password;
	const char* lwt_topic;
	const char* lwt_message;
} mqtt_client_config_t;

/**
 * @brief MQTT 브로커 연결
 * @param cfg MQTT 설정 구조체
 * @return 성공 시 true, 실패 시 false
 */
bool mqtt_connect(const mqtt_client_config_t& cfg);

/**
 * @brief MQTT 토픽 구독
 * @param topic 구독할 토픽 이름
 * @return 성공 시 true, 실패 시 false
 */
bool mqtt_subscribe(const mqtt_client_config_t& cfg, const char* topic);

/**
 * @brief MQTT 메시지 발행
 * @param topic 발행할 토픽 이름
 * @param message 발행할 메시지 내용
 * @return 성공 시 true, 실패 시 false
 */
bool mqtt_publish(const mqtt_client_config_t& cfg, const char* topic, const char* message);

/**
 * @brief 수신된 MQTT 메시지 확인 및 처리
 */
void check_mqtt_messages(const mqtt_client_config_t& cfg);

/**
 * @brief MQTT 연결 상태 확인
 * @return 연결되어 있으면 true, 아니면 false
 */
bool mqtt_is_connected(const mqtt_client_config_t& cfg);

/**
 * @brief MQTT 연결 해제
 */
void mqtt_disconnect(const mqtt_client_config_t& cfg);


/**
 * @brief MQTT 브로커 재연결 시도
 * @param mqtt_cfg MQTT 설정 구조체
 * @param esp_cfg ESP-01 설정 구조체
 * @return 성공 시 true, 실패 시 false
 */
bool mqtt_reconnect(const mqtt_client_config_t& mqtt_cfg, const esp01_config_t& esp_cfg);

#ifdef __cplusplus
}
#endif

#endif // MQTT_CLIENT_H
