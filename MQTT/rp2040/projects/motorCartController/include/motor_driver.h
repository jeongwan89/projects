#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <stdint.h>

#include "hardware/pwm.h"
#include "hardware/gpio.h"

/* ============================================
   DRV8871 모터 드라이버 설정 (4 Units)
   
   잡음 최소화 고려:
   - 각 모터마다 서로 다른 PWM 슬라이스 사용
   - GPIO 핀들을 물리적으로 분산 (PWM과 방향 핀 분리)
   - UART 핀(GPIO4, 5)과 분리
   - 고주파 영역(GPIO0-3) 회피
   
   GPIO 할당 맵:
   GPIO0-3:   [회피] 고주파 영역
   GPIO4-5:   [UART] ESP01 통신
   GPIO6:     Motor1 PWM (PWM3-A)
   GPIO7:     Motor1 IN1 (정회전 제어)
   GPIO8:     Motor2 PWM (PWM4-A)
   GPIO9:     Motor2 IN1 (정회전 제어)
   GPIO10:    Motor3 PWM (PWM5-A)
   GPIO11:    Motor3 IN1 (정회전 제어)
   GPIO12:    Motor1 IN2 (역회전 제어)
   GPIO13:    Motor2 IN2 (역회전 제어)
   GPIO14:    Motor4 PWM (PWM7-A)
   GPIO15:    Motor3 IN2 (역회전 제어)
   GPIO16:    Motor4 IN1 (정회전 제어)
   GPIO17:    Motor4 IN2 (역회전 제어)
   ============================================ */

/* ==================== 모터 1 ====================
   PWM: GPIO6 (PWM3 Channel A) - 속도 제어
   IN1: GPIO7 - 정회전 제어
   IN2: GPIO12 - 역회전 제어
   */
#define MOTOR1_PWM_PIN      6
#define MOTOR1_PWM_SLICE    pwm_gpio_to_slice_num(MOTOR1_PWM_PIN)
#define MOTOR1_PWM_CHAN     pwm_gpio_to_channel(MOTOR1_PWM_PIN)
#define MOTOR1_IN1_PIN      7       // 정회전
#define MOTOR1_IN2_PIN      12      // 역회전

/* ==================== 모터 2 ====================
   PWM: GPIO8 (PWM4 Channel A) - 속도 제어
   IN1: GPIO9 - 정회전 제어
   IN2: GPIO13 - 역회전 제어
   */
#define MOTOR2_PWM_PIN      8
#define MOTOR2_PWM_SLICE    pwm_gpio_to_slice_num(MOTOR2_PWM_PIN)
#define MOTOR2_PWM_CHAN     pwm_gpio_to_channel(MOTOR2_PWM_PIN)
#define MOTOR2_IN1_PIN      9       // 정회전
#define MOTOR2_IN2_PIN      13      // 역회전

/* ==================== 모터 3 ====================
   PWM: GPIO10 (PWM5 Channel A) - 속도 제어
   IN1: GPIO11 - 정회전 제어
   IN2: GPIO15 - 역회전 제어
   */
#define MOTOR3_PWM_PIN      10
#define MOTOR3_PWM_SLICE    pwm_gpio_to_slice_num(MOTOR3_PWM_PIN)
#define MOTOR3_PWM_CHAN     pwm_gpio_to_channel(MOTOR3_PWM_PIN)
#define MOTOR3_IN1_PIN      11      // 정회전
#define MOTOR3_IN2_PIN      15      // 역회전

/* ==================== 모터 4 ====================
   PWM: GPIO14 (PWM7 Channel A) - 속도 제어
   IN1: GPIO16 - 정회전 제어
   IN2: GPIO17 - 역회전 제어
   */
#define MOTOR4_PWM_PIN      14
#define MOTOR4_PWM_SLICE    pwm_gpio_to_slice_num(MOTOR4_PWM_PIN)
#define MOTOR4_PWM_CHAN     pwm_gpio_to_channel(MOTOR4_PWM_PIN)
#define MOTOR4_IN1_PIN      16      // 정회전
#define MOTOR4_IN2_PIN      17      // 역회전

/* 모터 ID 정의 */
typedef enum {
   MOTOR_1 = 0,
   MOTOR_2,
   MOTOR_3,
   MOTOR_4,
   MOTOR_COUNT
} motor_id_t;

#define PWM_FREQ_HZ         20000       // 20kHz (DRV8871 권장)
#define PWM_WRAP            2500        // Wrap value (계산: sys_clk_hz / (freq * PSDiv))
#define PWM_MAX_LEVEL       PWM_WRAP    // 최대 PWM 레벨 (100%)
#define PWM_MIN_LEVEL       0           // 최소 PWM 레벨 (0%)

/* 모터 방향 정의 */
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD = 1,
    MOTOR_BACKWARD = -1
} motor_direction_t;

/* 모터 상태 구조체 */
typedef struct {
    uint16_t pwm_level;         // PWM 레벨 (0 ~ PWM_MAX_LEVEL)
    motor_direction_t direction; // 모터 방향
} motor_state_t;

/* ============================================
   함수 프로토타입
   ============================================ */

/**
 * 모든 모터 드라이버 초기화
 * - PWM 신호 설정
 * - GPIO 핀 구성
 */
void motor_init_all(void);

/**
 * 특정 모터 초기화
 * @param motor_id: 모터 ID (MOTOR_1 ~ MOTOR_4)
 */
void motor_init(motor_id_t motor_id);

/**
 * 모터 정회전 (Forward)
 * @param motor_id: 모터 ID
 * @param speed: 속도 (0 ~ 100%)
 */
void motor_forward(motor_id_t motor_id, uint8_t speed);

/**
 * 모터 역회전 (Backward)
 * @param motor_id: 모터 ID
 * @param speed: 속도 (0 ~ 100%)
 */
void motor_backward(motor_id_t motor_id, uint8_t speed);

/**
 * 모터 속도 및 방향 설정
 * @param motor_id: 모터 ID
 * @param speed: 속도 (0 ~ 100%)
 * @param direction: 방향 (MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_STOP)
 */
void motor_set_speed_direction(motor_id_t motor_id, uint8_t speed, motor_direction_t direction);

/**
 * 모터 정지
 * @param motor_id: 모터 ID
 */
void motor_stop(motor_id_t motor_id);

/**
 * 모든 모터 정지
 */
void motor_stop_all(void);

/**
 * 모터 브레이크 (IN1=HIGH, IN2=HIGH)
 * @param motor_id: 모터 ID
 */
void motor_brake(motor_id_t motor_id);

/**
 * 모든 모터 브레이크
 */
void motor_brake_all(void);

/**
 * 비상 정지
 * - 4개 모터를 즉시 브레이크 상태로 전환
 */
void motor_emergency_stop(void);

/**
 * 모터 속도 증가
 * @param motor_id: 모터 ID
 * @param increment: 증가량 (%)
 */
void motor_speed_increment(motor_id_t motor_id, int8_t increment);

/**
 * 단일 모터 가감속 램프
 * @param motor_id: 모터 ID
 * @param target_speed: 목표 속도 (0 ~ 100%)
 * @param direction: 목표 방향
 * @param step_percent: 스텝 변화량 (%)
 * @param step_delay_ms: 스텝 지연 시간 (ms)
 */
void motor_ramp_to(motor_id_t motor_id,
                   uint8_t target_speed,
                   motor_direction_t direction,
                   uint8_t step_percent,
                   uint32_t step_delay_ms);

/**
 * 4개 모터 동기 제어 (동일 속도/방향)
 * @param speed: 속도 (0 ~ 100%)
 * @param direction: 방향
 */
void motor_sync_set_all(uint8_t speed, motor_direction_t direction);

/**
 * 4개 모터 동기 제어 (개별 속도/방향)
 * @param speeds: 모터별 속도 배열 [MOTOR_COUNT]
 * @param directions: 모터별 방향 배열 [MOTOR_COUNT]
 */
void motor_sync_set_array(const uint8_t speeds[MOTOR_COUNT],
                          const motor_direction_t directions[MOTOR_COUNT]);

/**
 * 4개 모터 동기 가감속
 * @param target_speeds: 모터별 목표 속도 배열 [MOTOR_COUNT]
 * @param directions: 모터별 목표 방향 배열 [MOTOR_COUNT]
 * @param step_percent: 스텝 변화량 (%)
 * @param step_delay_ms: 스텝 지연 시간 (ms)
 */
void motor_sync_ramp_array(const uint8_t target_speeds[MOTOR_COUNT],
                           const motor_direction_t directions[MOTOR_COUNT],
                           uint8_t step_percent,
                           uint32_t step_delay_ms);

/**
 * 모터 현재 상태 조회
 * @param motor_id: 모터 ID
 * @return 모터 상태 구조체 (속도, 방향)
 */
motor_state_t motor_get_state(motor_id_t motor_id);

/**
 * 모터 현재 속도 조회
 * @param motor_id: 모터 ID
 * @return 현재 속도 (%)
 */
uint8_t motor_get_speed(motor_id_t motor_id);

/**
 * 모터 현재 방향 조회
 * @param motor_id: 모터 ID
 * @return 모터 방향
 */
motor_direction_t motor_get_direction(motor_id_t motor_id);

/**
 * PWM 레벨을 속도(%)로 변환
 * @param level: PWM 레벨 (0 ~ PWM_MAX_LEVEL)
 * @return 속도 (0 ~ 100%)
 */
uint8_t pwm_level_to_percent(uint16_t level);

/**
 * 속도(%)를 PWM 레벨로 변환
 * @param percent: 속도 (0 ~ 100%)
 * @return PWM 레벨 (0 ~ PWM_MAX_LEVEL)
 */
uint16_t percent_to_pwm_level(uint8_t percent);

#endif // MOTOR_DRIVER_H
