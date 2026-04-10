#include "motor_driver.h"

#include <stdbool.h>
#include <stddef.h>

#include "pico/stdlib.h"

typedef struct {
    uint pwm_pin;
    uint in1_pin;
    uint in2_pin;
    uint slice;
    uint channel;
} motor_hw_t;

static const motor_hw_t g_motor_hw[MOTOR_COUNT] = {
    {MOTOR1_PWM_PIN, MOTOR1_IN1_PIN, MOTOR1_IN2_PIN, MOTOR1_PWM_SLICE, MOTOR1_PWM_CHAN},
    {MOTOR2_PWM_PIN, MOTOR2_IN1_PIN, MOTOR2_IN2_PIN, MOTOR2_PWM_SLICE, MOTOR2_PWM_CHAN},
    {MOTOR3_PWM_PIN, MOTOR3_IN1_PIN, MOTOR3_IN2_PIN, MOTOR3_PWM_SLICE, MOTOR3_PWM_CHAN},
    {MOTOR4_PWM_PIN, MOTOR4_IN1_PIN, MOTOR4_IN2_PIN, MOTOR4_PWM_SLICE, MOTOR4_PWM_CHAN},
};

static motor_state_t g_motor_state[MOTOR_COUNT];

static bool motor_id_valid(motor_id_t motor_id)
{
    return motor_id >= MOTOR_1 && motor_id < MOTOR_COUNT;
}

static void apply_pwm_level(motor_id_t motor_id, uint16_t level)
{
    const motor_hw_t *hw = &g_motor_hw[motor_id];
    pwm_set_chan_level(hw->slice, hw->channel, level);
    g_motor_state[motor_id].pwm_level = level;
}

static void apply_direction(motor_id_t motor_id, motor_direction_t direction)
{
    const motor_hw_t *hw = &g_motor_hw[motor_id];

    switch (direction) {
    case MOTOR_FORWARD:
        gpio_put(hw->in1_pin, 1);
        gpio_put(hw->in2_pin, 0);
        break;
    case MOTOR_BACKWARD:
        gpio_put(hw->in1_pin, 0);
        gpio_put(hw->in2_pin, 1);
        break;
    case MOTOR_STOP:
    default:
        gpio_put(hw->in1_pin, 0);
        gpio_put(hw->in2_pin, 0);
        break;
    }

    g_motor_state[motor_id].direction = direction;
}

static void apply_brake(motor_id_t motor_id)
{
    const motor_hw_t *hw = &g_motor_hw[motor_id];
    gpio_put(hw->in1_pin, 1);
    gpio_put(hw->in2_pin, 1);
    g_motor_state[motor_id].direction = MOTOR_STOP;
}

static uint8_t clamp_speed_percent(uint8_t speed)
{
    if (speed > 100) {
        return 100;
    }
    return speed;
}

void motor_init(motor_id_t motor_id)
{
    if (!motor_id_valid(motor_id)) {
        return;
    }

    const motor_hw_t *hw = &g_motor_hw[motor_id];

    gpio_set_function(hw->pwm_pin, GPIO_FUNC_PWM);
    gpio_init(hw->in1_pin);
    gpio_set_dir(hw->in1_pin, GPIO_OUT);
    gpio_put(hw->in1_pin, 0);

    gpio_init(hw->in2_pin);
    gpio_set_dir(hw->in2_pin, GPIO_OUT);
    gpio_put(hw->in2_pin, 0);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 2.5f); // 125MHz / 2.5 / 2500 = 20kHz
    pwm_config_set_wrap(&cfg, PWM_WRAP);
    pwm_init(hw->slice, &cfg, true);

    apply_pwm_level(motor_id, 0);
    apply_direction(motor_id, MOTOR_STOP);
}

void motor_init_all(void)
{
    for (motor_id_t i = MOTOR_1; i < MOTOR_COUNT; i++) {
        motor_init(i);
    }
}

void motor_set_speed_direction(motor_id_t motor_id, uint8_t speed, motor_direction_t direction)
{
    if (!motor_id_valid(motor_id)) {
        return;
    }

    if (direction == MOTOR_STOP || speed == 0) {
        apply_pwm_level(motor_id, 0);
        apply_direction(motor_id, MOTOR_STOP);
        return;
    }

    apply_direction(motor_id, direction);
    apply_pwm_level(motor_id, percent_to_pwm_level(speed));
}

void motor_forward(motor_id_t motor_id, uint8_t speed)
{
    motor_set_speed_direction(motor_id, speed, MOTOR_FORWARD);
}

void motor_backward(motor_id_t motor_id, uint8_t speed)
{
    motor_set_speed_direction(motor_id, speed, MOTOR_BACKWARD);
}

void motor_stop(motor_id_t motor_id)
{
    if (!motor_id_valid(motor_id)) {
        return;
    }

    apply_pwm_level(motor_id, 0);
    apply_direction(motor_id, MOTOR_STOP);
}

void motor_stop_all(void)
{
    for (motor_id_t i = MOTOR_1; i < MOTOR_COUNT; i++) {
        motor_stop(i);
    }
}

void motor_brake(motor_id_t motor_id)
{
    if (!motor_id_valid(motor_id)) {
        return;
    }

    apply_pwm_level(motor_id, 0);
    apply_brake(motor_id);
}

void motor_brake_all(void)
{
    for (motor_id_t i = MOTOR_1; i < MOTOR_COUNT; i++) {
        motor_brake(i);
    }
}

void motor_emergency_stop(void)
{
    motor_brake_all();
}

void motor_speed_increment(motor_id_t motor_id, int8_t increment)
{
    if (!motor_id_valid(motor_id)) {
        return;
    }

    int new_speed = (int)motor_get_speed(motor_id) + (int)increment;
    if (new_speed < 0) {
        new_speed = 0;
    } else if (new_speed > 100) {
        new_speed = 100;
    }

    motor_direction_t dir = motor_get_direction(motor_id);
    if (dir == MOTOR_STOP && new_speed > 0) {
        dir = MOTOR_FORWARD;
    }

    motor_set_speed_direction(motor_id, (uint8_t)new_speed, dir);
}

void motor_ramp_to(motor_id_t motor_id,
                   uint8_t target_speed,
                   motor_direction_t direction,
                   uint8_t step_percent,
                   uint32_t step_delay_ms)
{
    if (!motor_id_valid(motor_id)) {
        return;
    }

    if (direction == MOTOR_STOP || target_speed == 0) {
        motor_stop(motor_id);
        return;
    }

    uint8_t target = clamp_speed_percent(target_speed);
    uint8_t step = (step_percent == 0) ? 1 : step_percent;
    uint8_t current = motor_get_speed(motor_id);

    if (motor_get_direction(motor_id) != direction) {
        motor_set_speed_direction(motor_id, 0, direction);
        current = 0;
    }

    while (current != target) {
        if (current < target) {
            uint16_t next = (uint16_t)current + (uint16_t)step;
            current = (next > target) ? target : (uint8_t)next;
        } else {
            int next = (int)current - (int)step;
            current = (next < (int)target) ? target : (uint8_t)next;
        }

        motor_set_speed_direction(motor_id, current, direction);
        if (step_delay_ms > 0) {
            sleep_ms(step_delay_ms);
        }
    }
}

void motor_sync_set_all(uint8_t speed, motor_direction_t direction)
{
    for (motor_id_t i = MOTOR_1; i < MOTOR_COUNT; i++) {
        motor_set_speed_direction(i, speed, direction);
    }
}

void motor_sync_set_array(const uint8_t speeds[MOTOR_COUNT],
                          const motor_direction_t directions[MOTOR_COUNT])
{
    if (speeds == NULL || directions == NULL) {
        return;
    }

    for (motor_id_t i = MOTOR_1; i < MOTOR_COUNT; i++) {
        motor_set_speed_direction(i, speeds[i], directions[i]);
    }
}

void motor_sync_ramp_array(const uint8_t target_speeds[MOTOR_COUNT],
                           const motor_direction_t directions[MOTOR_COUNT],
                           uint8_t step_percent,
                           uint32_t step_delay_ms)
{
    if (target_speeds == NULL || directions == NULL) {
        return;
    }

    uint8_t step = (step_percent == 0) ? 1 : step_percent;
    uint8_t current[MOTOR_COUNT];
    uint8_t target[MOTOR_COUNT];
    bool done = false;

    for (motor_id_t i = MOTOR_1; i < MOTOR_COUNT; i++) {
        current[i] = motor_get_speed(i);
        target[i] = clamp_speed_percent(target_speeds[i]);

        if (directions[i] == MOTOR_STOP || target[i] == 0) {
            current[i] = 0;
            target[i] = 0;
            motor_stop(i);
        } else if (motor_get_direction(i) != directions[i]) {
            current[i] = 0;
            motor_set_speed_direction(i, 0, directions[i]);
        }
    }

    while (!done) {
        done = true;

        for (motor_id_t i = MOTOR_1; i < MOTOR_COUNT; i++) {
            if (directions[i] == MOTOR_STOP || target[i] == 0) {
                continue;
            }

            if (current[i] < target[i]) {
                uint16_t next = (uint16_t)current[i] + (uint16_t)step;
                current[i] = (next > target[i]) ? target[i] : (uint8_t)next;
                done = false;
            } else if (current[i] > target[i]) {
                int next = (int)current[i] - (int)step;
                current[i] = (next < (int)target[i]) ? target[i] : (uint8_t)next;
                done = false;
            }

            motor_set_speed_direction(i, current[i], directions[i]);
        }

        if (!done && step_delay_ms > 0) {
            sleep_ms(step_delay_ms);
        }
    }
}

motor_state_t motor_get_state(motor_id_t motor_id)
{
    static const motor_state_t k_default_state = {0, MOTOR_STOP};

    if (!motor_id_valid(motor_id)) {
        return k_default_state;
    }

    return g_motor_state[motor_id];
}

uint8_t motor_get_speed(motor_id_t motor_id)
{
    if (!motor_id_valid(motor_id)) {
        return 0;
    }

    return pwm_level_to_percent(g_motor_state[motor_id].pwm_level);
}

motor_direction_t motor_get_direction(motor_id_t motor_id)
{
    if (!motor_id_valid(motor_id)) {
        return MOTOR_STOP;
    }

    return g_motor_state[motor_id].direction;
}

uint8_t pwm_level_to_percent(uint16_t level)
{
    if (level >= PWM_MAX_LEVEL) {
        return 100;
    }

    return (uint8_t)((level * 100u) / PWM_MAX_LEVEL);
}

uint16_t percent_to_pwm_level(uint8_t percent)
{
    if (percent >= 100) {
        return PWM_MAX_LEVEL;
    }

    return (uint16_t)((percent * PWM_MAX_LEVEL) / 100u);
}
