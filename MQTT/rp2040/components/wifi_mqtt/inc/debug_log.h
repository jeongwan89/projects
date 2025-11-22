#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H


#include <stdio.h>
#include <stdbool.h>
#include "mqtt_client.h"

// 디버그 로그 모드 설정
#define DEBUG_LOG_USB    1  // USB로 출력
#define DEBUG_LOG_MQTT   2  // MQTT로 전송
#define DEBUG_LOG_BOTH   3  // 둘 다

#ifndef DEBUG_LOG_MODE
#define DEBUG_LOG_MODE DEBUG_LOG_BOTH
#endif

/**
 * @brief 디버그 로그 초기화
 */
void debug_log_init(void);

/**
 * @brief 디버그 메시지 출력 (printf 대체용)
 * @param format printf 형식 문자열
 */
void debug_printf(const mqtt_client_config_t& cfg, const char* format, ...);

/**
 * @brief MQTT를 통한 로그 전송 활성화
 * @param enable true: 활성화, false: 비활성화
 */
void debug_mqtt_enable(bool enable);

#endif // DEBUG_LOG_H
