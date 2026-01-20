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
    float value;        // 센서 원본 값
    float offset;       // 보정값 (오프셋)
    bool valid;         // 데이터 유효성
} DisplayData;

// 토픽-디스플레이 매핑 구조체
typedef struct {
    const char* topic;
    int display_idx;
} TopicMapping;

// 전역 변수 선언
extern TM1637Display* displays[NUM_DISPLAYS];
extern DisplayData display_data[NUM_DISPLAYS];
extern const TopicMapping topic_map[NUM_DISPLAYS];

/**
 * @brief MQTT 재연결 후 초기화 작업 수행
 * 
 * MQTT 브로커 재연결 시 필요한 모든 초기화 작업을 수행합니다:
 * - 상태 토픽에 online 메시지 발행
 * - 센서 토픽 재구독 (GH1~GH4)
 * - 보정값 토픽 재구독 (GH1~GH4)
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
    
    // 센서 데이터 토픽 구독 (GH1~GH4)
    const char* sensor_topics[] = {
        TOPIC_GH1_TEMP, TOPIC_GH1_HUM,
        TOPIC_GH2_TEMP, TOPIC_GH2_HUM,
        TOPIC_GH3_TEMP, TOPIC_GH3_HUM,
        TOPIC_GH4_TEMP, TOPIC_GH4_HUM
    };
    
    for (size_t i = 0; i < sizeof(sensor_topics) / sizeof(sensor_topics[0]); i++) {
        if (!mqtt_subscribe(mqtt, sensor_topics[i], 0)) {
            printf("[오류] 센서 토픽 재구독 실패: %s\n", sensor_topics[i]);
            return false;
        }
        printf("[MQTT] 센서 토픽 재구독 완료: %s\n", sensor_topics[i]);
    }
    
    // 보정값 토픽 구독 (GH1~GH4)
    const char* offset_topics[] = {
        TOPIC_GH1_TEMP_OFFSET, TOPIC_GH1_HUM_OFFSET,
        TOPIC_GH2_TEMP_OFFSET, TOPIC_GH2_HUM_OFFSET,
        TOPIC_GH3_TEMP_OFFSET, TOPIC_GH3_HUM_OFFSET,
        TOPIC_GH4_TEMP_OFFSET, TOPIC_GH4_HUM_OFFSET
    };
    
    for (size_t i = 0; i < sizeof(offset_topics) / sizeof(offset_topics[0]); i++) {
        if (!mqtt_subscribe(mqtt, offset_topics[i], 0)) {
            printf("[오류] 보정값 토픽 재구독 실패: %s\n", offset_topics[i]);
            return false;
        }
        printf("[MQTT] 보정값 토픽 재구독 완료: %s\n", offset_topics[i]);
    }
    
    printf("[MQTT] 재연결 후 초기화 완료\n");
    return true;
}

/**
 * @brief MQTT 메시지에서 float 값 파싱 (범위 검증)
 * 
 * 센서 값의 범위를 검증하여 유효한 범위 내의 값만 반환합니다.
 * 범위 초과 시 0.0f 반환
 * 
 * @param message MQTT 메시지 문자열
 * @return float 파싱된 실수 값 (범위: SENSOR_VALUE_MIN ~ SENSOR_VALUE_MAX)
 */
inline float parse_float_from_message(const char* message) {
    float value = atof(message);
    if (value < SENSOR_VALUE_MIN || value > SENSOR_VALUE_MAX) {
        printf("[경고] 센서 값 범위 초과: %.1f (허용: %.1f ~ %.1f)\n", 
               value, SENSOR_VALUE_MIN, SENSOR_VALUE_MAX);
        return 0.0f;
    }
    return value;
}

/**
 * @brief MQTT 메시지 처리 (센서 데이터 및 보정값 업데이트 - 테이블 기반)
 * 
 * 토픽-디스플레이 매핑 테이블을 사용하여 동적으로 업데이트합니다.
 * - 센서 값: display_data[idx].value 업데이트
 * - 보정값: display_data[idx].offset 업데이트
 * 
 * @param topic MQTT 토픽
 * @param message MQTT 메시지
 */
inline void process_mqtt_message(const char* topic, const char* message) {
    printf("[수신] %s: %s\n", topic, message);
    
    float value = parse_float_from_message(message);
    
    // 센서 값 토픽 처리
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        if (strcmp(topic, topic_map[i].topic) == 0) {
            int idx = topic_map[i].display_idx;
            display_data[idx].value = value;
            display_data[idx].valid = true;
            return;
        }
    }
    
    // 보정값 토픽 처리
    const char* offset_topics[] = {
        TOPIC_GH1_TEMP_OFFSET, TOPIC_GH1_HUM_OFFSET,
        TOPIC_GH2_TEMP_OFFSET, TOPIC_GH2_HUM_OFFSET,
        TOPIC_GH3_TEMP_OFFSET, TOPIC_GH3_HUM_OFFSET,
        TOPIC_GH4_TEMP_OFFSET, TOPIC_GH4_HUM_OFFSET
    };
    
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        if (strcmp(topic, offset_topics[i]) == 0) {
            display_data[i].offset = value;
            printf("[OK] 보정값 업데이트: 디스플레이 %d, 오프셋=%.1f\n", i, value);
            return;
        }
    }
    
    printf("[경고] 알 수 없는 토픽: %s\n", topic);
}

/**
 * @brief 시스템 정리 함수 (프로그램 종료 시)
 * 
 * 동적 할당된 메모리와 리소스를 정리합니다.
 */
inline void cleanup_resources() {
    printf("\n[정보] 리소스 정리 중...\n");
    
    // TM1637 디스플레이 정리
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        if (displays[i] != nullptr) {
            displays[i]->clear();
            delete displays[i];
            displays[i] = nullptr;
        }
    }
    
    printf("[OK] 리소스 정리 완료\n");
}

/**
 * @brief 모든 디스플레이 업데이트 (보정값 적용)
 * 
 * 8개의 모든 TM1637 디스플레이를 현재 센서 데이터 + 보정값으로 업데이트합니다.
 * - 최종값 = 센서값 + 보정값
 * - 짝수 인덱스: 온도 표시
 * - 홀수 인덱스: 습도 표시
 * - 데이터 없을 때: 화면 지우기
 */
inline void update_all_displays() {
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        if (display_data[i].valid) {
            // 최종 표시값 = 센서값 + 보정값
            float corrected_value = display_data[i].value + display_data[i].offset;
            
            // 짝수 인덱스 = 온도, 홀수 인덱스 = 습도
            if (i % 2 == 0) {
                displays[i]->showTemperature(corrected_value);
            } else {
                displays[i]->showHumidity(corrected_value);
            }
        } else {
            // 데이터 없을 때 화면 지우기
            displays[i]->clear();
        }
    }
}

#endif // MAIN_H
