#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <stdbool.h>
#include "mqtt_client.h"
#include "tm1637.h"
#include "config.h"

/**
 * @brief 센서 데이터 저장 구조체 (8개 디스플레이용)
 */
typedef struct {
    float value;      // 센서 값
    bool valid;       // 데이터 유효성
} DisplayData;

// 전역 변수 선언
extern TM1637Display* displays[NUM_DISPLAYS];
extern DisplayData display_data[NUM_DISPLAYS];

/**
 * @brief MQTT 재연결 후 초기화 작업 수행
 * 
 * MQTT 브로커 재연결 시 필요한 모든 초기화 작업을 수행합니다:
 * - 상태 토픽에 online 메시지 발행
 * - 센서 토픽 재구독 (GH1~GH4)
 * 
 * @param mqtt MQTT 클라이언트 구조체
 * @return true 모든 초기화 작업 성공
 * @return false 초기화 작업 중 하나라도 실패
 */
inline bool mqtt_reinitialize_after_reconnect(MqttClient& mqtt) {
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
        TOPIC_GH4_TEMP, TOPIC_GH4_HUM
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
 * 
 * @param message MQTT 메시지 문자열
 * @return float 파싱된 실수 값
 */
inline float parse_float_from_message(const char* message) {
    return atof(message);
}

/**
 * @brief MQTT 메시지 처리 (센서 데이터 업데이트)
 * 
 * 수신한 MQTT 토픽에 따라 해당 디스플레이 데이터를 업데이트합니다.
 * 
 * @param topic MQTT 토픽
 * @param message MQTT 메시지
 */
inline void process_mqtt_message(const char* topic, const char* message) {
    printf("[수신] %s: %s\n", topic, message);
    
    float value = parse_float_from_message(message);
    
    // 온도/습도 데이터 파싱 및 해당 디스플레이 업데이트
    if (strcmp(topic, TOPIC_GH1_TEMP) == 0) {
        display_data[DISPLAY_GH1_TEMP].value = value;
        display_data[DISPLAY_GH1_TEMP].valid = true;
    }
    else if (strcmp(topic, TOPIC_GH1_HUM) == 0) {
        display_data[DISPLAY_GH1_HUM].value = value;
        display_data[DISPLAY_GH1_HUM].valid = true;
    }
    else if (strcmp(topic, TOPIC_GH2_TEMP) == 0) {
        display_data[DISPLAY_GH2_TEMP].value = value;
        display_data[DISPLAY_GH2_TEMP].valid = true;
    }
    else if (strcmp(topic, TOPIC_GH2_HUM) == 0) {
        display_data[DISPLAY_GH2_HUM].value = value;
        display_data[DISPLAY_GH2_HUM].valid = true;
    }
    else if (strcmp(topic, TOPIC_GH3_TEMP) == 0) {
        display_data[DISPLAY_GH3_TEMP].value = value;
        display_data[DISPLAY_GH3_TEMP].valid = true;
    }
    else if (strcmp(topic, TOPIC_GH3_HUM) == 0) {
        display_data[DISPLAY_GH3_HUM].value = value;
        display_data[DISPLAY_GH3_HUM].valid = true;
    }
    else if (strcmp(topic, TOPIC_GH4_TEMP) == 0) {
        display_data[DISPLAY_GH4_TEMP].value = value;
        display_data[DISPLAY_GH4_TEMP].valid = true;
    }
    else if (strcmp(topic, TOPIC_GH4_HUM) == 0) {
        display_data[DISPLAY_GH4_HUM].value = value;
        display_data[DISPLAY_GH4_HUM].valid = true;
    }
}

/**
 * @brief 모든 디스플레이 업데이트
 * 
 * 8개의 모든 TM1637 디스플레이를 현재 센서 데이터로 업데이트합니다.
 * - 짝수 인덱스: 온도 표시
 * - 홀수 인덱스: 습도 표시
 * - 데이터 없을 때: 화면 지우기
 */
inline void update_all_displays() {
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        if (display_data[i].valid) {
            // 짝수 인덱스 = 온도, 홀수 인덱스 = 습도
            if (i % 2 == 0) {
                displays[i]->showTemperature(display_data[i].value);
            } else {
                displays[i]->showHumidity(display_data[i].value);
            }
        } else {
            // 데이터 없을 때 화면 지우기
            displays[i]->clear();
        }
    }
}

#endif // MAIN_H
