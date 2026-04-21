#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "motor_driver.h"
#include "esp01_adapter.h"
#include "mqtt_adapter.h"
#include "ultrasonic.h"

// Emergency switch: active-low input with pull-up.
#define EMERGENCY_SW_GPIO            22
#define EMERGENCY_SW_DEBOUNCE_MS     50
#define EMERGENCY_RELEASE_STABLE_MS  1000
#define MOTOR_STATUS_PUBLISH_MS      1000
#define EMERGENCY_SWITCH_TOPIC       "Cart/1/EmergencySwitch"
#define EMERGENCY_LATCH_TOPIC        "Cart/1/EmergencyLatch"

#define WIFI_SSID                    "FarmMain5G"
#define WIFI_PASSWORD                "wweerrtt"

// ESP01 reset pin (active-low), separated from UART/motor/emergency pins.
#define ESP01_RST_GPIO               18

// HC-SR04 x2 pin reservation (4 GPIOs)
#define US1_TRIG_GPIO                12
#define US1_ECHO_GPIO                13
#define US2_TRIG_GPIO                14
#define US2_ECHO_GPIO                15

#define ULTRASONIC_POLL_MS           100
#define ULTRASONIC_STOP_CM           100.0f
#define ULTRASONIC_CLEAR_CM          120.0f

#define ULTRASONIC1_DISTANCE_TOPIC   "Cart/1/Ultrasonic1/DistanceCm"
#define ULTRASONIC2_DISTANCE_TOPIC   "Cart/1/Ultrasonic2/DistanceCm"
#define ULTRASONIC1_VALID_TOPIC      "Cart/1/Ultrasonic1/Valid"
#define ULTRASONIC2_VALID_TOPIC      "Cart/1/Ultrasonic2/Valid"
#define ULTRASONIC_OBSTACLE_TOPIC    "Cart/1/Ultrasonic/Obstacle"

static volatile bool g_emergency_irq_pending = false;
static volatile bool g_emergency_latched = false;
static volatile uint32_t g_last_emergency_irq_ms = 0;
static bool g_publish_status_now = false;
static bool g_emergency_status_inited = false;
static bool g_last_switch_pressed = false;
static bool g_last_latched_state = false;
static bool g_obstacle_latched = false;
static uint32_t g_last_ultrasonic_poll_ms = 0;
static bool g_last_us1_valid = false;
static bool g_last_us2_valid = false;
static float g_last_us1_cm = 0.0f;
static float g_last_us2_cm = 0.0f;
static bool g_ultrasonic_status_inited = false;

static ultrasonic_sensor_t g_us1;
static ultrasonic_sensor_t g_us2;

static uint8_t g_target_speed[MOTOR_COUNT] = {0};
static motor_direction_t g_target_dir[MOTOR_COUNT] = {
    MOTOR_STOP,
    MOTOR_STOP,
    MOTOR_STOP,
    MOTOR_STOP,
};

static void apply_motor_target(motor_id_t motor_id)
{
    if (g_obstacle_latched) {
        motor_stop(motor_id);
        g_publish_status_now = true;
        return;
    }

    if (g_target_dir[motor_id] == MOTOR_STOP || g_target_speed[motor_id] == 0) {
        motor_stop(motor_id);
        g_publish_status_now = true;
        return;
    }

    motor_set_speed_direction(motor_id, g_target_speed[motor_id], g_target_dir[motor_id]);
    g_publish_status_now = true;
}

static const char *dir_to_text(motor_direction_t dir)
{
    if (dir == MOTOR_FORWARD) {
        return "FW";
    }
    if (dir == MOTOR_BACKWARD) {
        return "BW";
    }
    return "Stop";
}

static void publish_motor_states(void)
{
    char topic[64];
    char payload[16];

    for (int i = 0; i < MOTOR_COUNT; i++) {
        motor_id_t motor_id = (motor_id_t)i;

        snprintf(topic, sizeof(topic), "Cart/1/Motor%d/StateDir", i + 1);
        (void)mqtt_publish_text(topic, dir_to_text(motor_get_direction(motor_id)), 1, 0);

        snprintf(topic, sizeof(topic), "Cart/1/Motor%d/StateSpeed", i + 1);
        snprintf(payload, sizeof(payload), "%u", (unsigned int)motor_get_speed(motor_id));
        (void)mqtt_publish_text(topic, payload, 1, 0);
    }
}

static void publish_emergency_state(bool force)
{
    // Active-low switch: LOW means pressed.
    bool switch_pressed = !gpio_get(EMERGENCY_SW_GPIO);
    bool latched = g_emergency_latched;

    if (!force && g_emergency_status_inited &&
        switch_pressed == g_last_switch_pressed &&
        latched == g_last_latched_state) {
        return;
    }

    (void)mqtt_publish_text(
        EMERGENCY_SWITCH_TOPIC,
        switch_pressed ? "Pressed" : "Released",
        1,
        0);

    (void)mqtt_publish_text(
        EMERGENCY_LATCH_TOPIC,
        latched ? "Latched" : "Normal",
        1,
        0);

    g_last_switch_pressed = switch_pressed;
    g_last_latched_state = latched;
    g_emergency_status_inited = true;
}

static bool parse_dir_payload(const char *payload, motor_direction_t *dir)
{
    if (strcmp(payload, "FW") == 0) {
        *dir = MOTOR_FORWARD;
        return true;
    }
    if (strcmp(payload, "BW") == 0) {
        *dir = MOTOR_BACKWARD;
        return true;
    }
    if (strcmp(payload, "Stop") == 0) {
        *dir = MOTOR_STOP;
        return true;
    }
    if (strcmp(payload, "Brake") == 0) {
        *dir = MOTOR_STOP;
        return true;
    }

    return false;
}

static void process_mqtt_motor_command(const char *topic, const char *payload)
{
    int motor_number = 0;
    char field[16] = {0};

    if (sscanf(topic, "Cart/1/Motor%d/%15s", &motor_number, field) != 2) {
        return;
    }

    if (motor_number < 1 || motor_number > MOTOR_COUNT) {
        return;
    }

    motor_id_t motor_id = (motor_id_t)(motor_number - 1);

    if (strcmp(field, "Dir") == 0) {
        motor_direction_t dir;
        if (!parse_dir_payload(payload, &dir)) {
            printf("[MQTT] Invalid Dir payload for Motor%d: %s\n", motor_number, payload);
            return;
        }

        if (strcmp(payload, "Brake") == 0) {
            motor_brake(motor_id);
            g_target_dir[motor_id] = MOTOR_STOP;
            g_target_speed[motor_id] = 0;
            g_publish_status_now = true;
            printf("[MQTT] Motor%d -> Brake\n", motor_number);
            return;
        }

        g_target_dir[motor_id] = dir;
        apply_motor_target(motor_id);
        printf("[MQTT] Motor%d Dir=%s\n", motor_number, payload);
        return;
    }

    if (strcmp(field, "Speed") == 0) {
        char *endptr = NULL;
        long speed = strtol(payload, &endptr, 10);
        if (endptr == payload || *endptr != '\0' || speed < 0 || speed > 100) {
            printf("[MQTT] Invalid Speed payload for Motor%d: %s\n", motor_number, payload);
            return;
        }

        g_target_speed[motor_id] = (uint8_t)speed;
        apply_motor_target(motor_id);
        printf("[MQTT] Motor%d Speed=%ld\n", motor_number, speed);
    }
}

static void safe_rearm_after_emergency(void)
{
    // Keep outputs in safe state and reinitialize motor driver before running again.
    motor_stop_all();
    g_publish_status_now = true;
    sleep_ms(100);
    motor_init_all();
    g_publish_status_now = true;
    sleep_ms(50);
}

static bool ultrasonic_safety_init(void)
{
    bool ok1 = ultrasonic_init(&g_us1, US1_TRIG_GPIO, US1_ECHO_GPIO);
    bool ok2 = ultrasonic_init(&g_us2, US2_TRIG_GPIO, US2_ECHO_GPIO);

    return ok1 && ok2;
}

static void publish_ultrasonic_state(bool force)
{
    char payload[24];

    if (!force && g_ultrasonic_status_inited == false) {
        return;
    }

    if (g_last_us1_valid) {
        snprintf(payload, sizeof(payload), "%.1f", g_last_us1_cm);
        (void)mqtt_publish_text(ULTRASONIC1_DISTANCE_TOPIC, payload, 1, 0);
    }
    (void)mqtt_publish_text(ULTRASONIC1_VALID_TOPIC, g_last_us1_valid ? "1" : "0", 1, 0);

    if (g_last_us2_valid) {
        snprintf(payload, sizeof(payload), "%.1f", g_last_us2_cm);
        (void)mqtt_publish_text(ULTRASONIC2_DISTANCE_TOPIC, payload, 1, 0);
    }
    (void)mqtt_publish_text(ULTRASONIC2_VALID_TOPIC, g_last_us2_valid ? "1" : "0", 1, 0);

    (void)mqtt_publish_text(ULTRASONIC_OBSTACLE_TOPIC, g_obstacle_latched ? "Near" : "Clear", 1, 0);
    g_ultrasonic_status_inited = true;
}

static void latch_obstacle_stop(float d1_cm, bool v1, float d2_cm, bool v2)
{
    if (g_obstacle_latched) {
        return;
    }

    g_obstacle_latched = true;
    for (int i = 0; i < MOTOR_COUNT; i++) {
        g_target_speed[i] = 0;
        g_target_dir[i] = MOTOR_STOP;
    }
    motor_stop_all();
    g_publish_status_now = true;
    (void)mqtt_publish_device_status("Obstacle", 1);

    printf("[ULTRASONIC] Obstacle detected. US1=%s%.1fcm, US2=%s%.1fcm, motors stopped.\n",
           v1 ? "" : "N/A ",
           v1 ? d1_cm : 0.0f,
           v2 ? "" : "N/A ",
           v2 ? d2_cm : 0.0f);
}

static void poll_ultrasonic_safety(void)
{
    uint32_t now_ms = to_ms_since_boot(get_absolute_time());
    if ((now_ms - g_last_ultrasonic_poll_ms) < ULTRASONIC_POLL_MS) {
        return;
    }
    g_last_ultrasonic_poll_ms = now_ms;

    float d1_cm = 0.0f;
    float d2_cm = 0.0f;
    bool v1 = ultrasonic_read_cm(&g_us1, &d1_cm);
    bool v2 = ultrasonic_read_cm(&g_us2, &d2_cm);

    g_last_us1_valid = v1;
    g_last_us2_valid = v2;
    g_last_us1_cm = d1_cm;
    g_last_us2_cm = d2_cm;

    if (!v1 && !v2) {
        publish_ultrasonic_state(true);
        return;
    }

    bool near = (v1 && d1_cm <= ULTRASONIC_STOP_CM) ||
                (v2 && d2_cm <= ULTRASONIC_STOP_CM);
    bool clear = (!v1 || d1_cm >= ULTRASONIC_CLEAR_CM) &&
                 (!v2 || d2_cm >= ULTRASONIC_CLEAR_CM);

    if (near) {
        latch_obstacle_stop(d1_cm, v1, d2_cm, v2);
    } else if (g_obstacle_latched && clear) {
        g_obstacle_latched = false;
        (void)mqtt_publish_device_status("Online", 1);
        printf("[ULTRASONIC] Obstacle cleared. Motion commands re-enabled.\n");
    }

    publish_ultrasonic_state(true);
}

static void emergency_switch_irq_callback(uint gpio, uint32_t events)
{
    if (gpio != EMERGENCY_SW_GPIO) {
        return;
    }

    if ((events & GPIO_IRQ_EDGE_FALL) == 0) {
        return;
    }

    uint32_t now_ms = to_ms_since_boot(get_absolute_time());
    if ((now_ms - g_last_emergency_irq_ms) < EMERGENCY_SW_DEBOUNCE_MS) {
        return;
    }

    g_last_emergency_irq_ms = now_ms;
    g_emergency_irq_pending = true;
}

static void emergency_switch_init(void)
{
    gpio_init(EMERGENCY_SW_GPIO);
    gpio_set_dir(EMERGENCY_SW_GPIO, GPIO_IN);
    gpio_pull_up(EMERGENCY_SW_GPIO);

    // Falling edge: switch pressed (GPIO pulled down to GND).
    gpio_set_irq_enabled_with_callback(
        EMERGENCY_SW_GPIO,
        GPIO_IRQ_EDGE_FALL,
        true,
        &emergency_switch_irq_callback);
}

int main(void)
{
    // USB 표준 출력 초기화
    stdio_init_all();
    
    motor_init_all();
    emergency_switch_init();
    if (!ultrasonic_safety_init()) {
        printf("[ULTRASONIC] Sensor init failed. System stays in safe idle mode.\n");
        while (1) {
            motor_stop_all();
            sleep_ms(200);
        }
    }
    
    printf("\n========================================\n");
    printf("Motor Cart Controller - RP2040 Pico\n");
    printf("========================================\n");
    printf("System started successfully!\n\n");
    printf("ESP01 reset configured on GPIO%d (active-low).\n", ESP01_RST_GPIO);
    printf("Motor driver initialized (4 units).\n");
    printf("Emergency switch IRQ enabled on GPIO%d (active-low).\n", EMERGENCY_SW_GPIO);
    printf("HC-SR04 reserved pins: US1(T%d/E%d), US2(T%d/E%d).\n",
           US1_TRIG_GPIO, US1_ECHO_GPIO, US2_TRIG_GPIO, US2_ECHO_GPIO);
    
    // Wi-Fi 연결 먼저 수행
    if (!esp01_init_and_connect_wifi(ESP01_RST_GPIO, WIFI_SSID, WIFI_PASSWORD)) {
        printf("[WIFI] Connection failed. System stays in safe idle mode.\n");
        while (1) {
            if (g_emergency_irq_pending) {
                g_emergency_irq_pending = false;
                g_emergency_latched = true;
                motor_emergency_stop();
            }
            sleep_ms(100);
        }
    }

    // MqttClient 구조체 기반 브로커 연결
    if (!mqtt_init_and_connect_default()) {
        printf("[MQTT] Broker connection failed. System stays in safe idle mode.\n");
        while (1) {
            if (g_emergency_irq_pending) {
                g_emergency_irq_pending = false;
                g_emergency_latched = true;
                motor_emergency_stop();
            }
            sleep_ms(100);
        }
    }

    if (!mqtt_subscribe_motor_topics()) {
        printf("[MQTT] Topic subscribe failed. System stays in safe idle mode.\n");
        while (1) {
            if (g_emergency_irq_pending) {
                g_emergency_irq_pending = false;
                g_emergency_latched = true;
                motor_emergency_stop();
            }
            sleep_ms(100);
        }
    }

    (void)mqtt_publish_device_status("Online", 1);
    
    printf("\nWaiting for MQTT-driven commands...\n");

    uint32_t release_stable_start_ms = 0;
    uint32_t last_status_publish_ms = 0;
    g_publish_status_now = true;
    publish_emergency_state(true);
    publish_ultrasonic_state(true);
    
    // 메인 루프
    while (1) {
        if (g_emergency_irq_pending) {
            g_emergency_irq_pending = false;
            g_emergency_latched = true;
            release_stable_start_ms = 0;
            motor_emergency_stop();
            g_publish_status_now = true;
            (void)mqtt_publish_device_status("Emergency", 1);
            printf("[EMERGENCY] Switch triggered on GPIO%d, all motors braked.\n", EMERGENCY_SW_GPIO);
        }

        if (g_emergency_latched) {
            // Released state is HIGH because of internal pull-up.
            bool released = gpio_get(EMERGENCY_SW_GPIO);
            uint32_t now_ms = to_ms_since_boot(get_absolute_time());

            if (released) {
                if (release_stable_start_ms == 0) {
                    release_stable_start_ms = now_ms;
                } else if ((now_ms - release_stable_start_ms) >= EMERGENCY_RELEASE_STABLE_MS) {
                    safe_rearm_after_emergency();
                    g_emergency_latched = false;
                    release_stable_start_ms = 0;
                    (void)mqtt_publish_device_status("Online", 1);
                    printf("[EMERGENCY] Switch released. Safe rearm completed, restarting control loop.\n");
                }
            } else {
                release_stable_start_ms = 0;
            }

            publish_emergency_state(false);

            sleep_ms(20);
            continue;
        }

        poll_ultrasonic_safety();
        if (g_obstacle_latched) {
            publish_emergency_state(false);
            sleep_ms(20);
            continue;
        }

        mqtt_run_maintenance();

        char topic[128] = {0};
        char message[128] = {0};
        if (mqtt_poll_message(topic, sizeof(topic), message, sizeof(message))) {
            process_mqtt_motor_command(topic, message);
        }

        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        if (g_publish_status_now || (now_ms - last_status_publish_ms) >= MOTOR_STATUS_PUBLISH_MS) {
            publish_motor_states();
            publish_ultrasonic_state(true);
            last_status_publish_ms = now_ms;
            g_publish_status_now = false;
        }

        publish_emergency_state(false);

        sleep_ms(10);
    }
    
    return 0;
}
