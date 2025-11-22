
#include <stdio.h>
#include "pico/stdlib.h"
#include "uart_comm.h"
#include "esp01.h"
#include "mqtt_client.h"
#include "serial_bridge.h"

int main(void) {

    stdio_init_all();

    // 설정 구조체 선언 및 초기화
    uart_config_t uart_cfg = {
        .uart_id = uart1,
        .baud_rate = 115200,
        .tx_pin = 4,
        .rx_pin = 5
    };
    esp01_config_t esp_cfg = {
        .rst_pin = 3,
        .reset_delay_ms = 5000,
        .ssid = "FarmMain",
        .password = "wweerrtt"
    };
    mqtt_client_config_t mqtt_cfg = {
        .broker = "192.168.0.24",
        .port = 1883,
        .client_id = "rp2040_client",
        .username = "farmmain",
        .password = "eerrtt",
        .topic_alive = "test/rp2040/alive",
        .topic_sensor = "test/rp2040/sensor",
        .topic_control = "test/rp2040/control",
        .topic_status = "test/rp2040/status",
        .lwt_message = "offline"
    };

    // USB CDC가 연결될 때까지 대기 (최대 5초)
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while (!stdio_usb_connected() && (to_ms_since_boot(get_absolute_time()) - start < 5000)) {
        sleep_ms(100);
    }
    sleep_ms(500);

    printf("\n\n=== RP2040 ESP-01 MQTT 시작 ===\n");

    // UART 초기화
    uart_init_esp01(&uart_cfg);

    // ESP-01 초기화 및 WiFi 연결 루프 (무한 재시도)
    bool wifi_ok = false;
    int retry_count = 0;
    while (!wifi_ok) {
        if (retry_count > 0) {
            printf("\n=== 전체 재시도 %d ===\n", retry_count + 1);
        }
        esp01_hardware_reset(&esp_cfg);
        if (!esp01_init(&esp_cfg)) {
            printf("ESP-01 초기화 실패\n");
            printf("시리얼 브리지 모드로 전환합니다...\n");
            serial_bridge_mode();
        }
        if (wifi_connect(&esp_cfg)) {
            wifi_ok = true;
            break;
        }
        printf("WiFi 연결 실패\n");
        printf("ESP-01을 리셋하고 처음부터 다시 시도합니다...\n");
        sleep_ms(2000);
        retry_count++;
    }

    sleep_ms(2000);

    // MQTT 연결
    if (!mqtt_connect(&mqtt_cfg)) {
        printf("MQTT 연결 실패\n");
        return -1;
    }

    // 제어 토픽 구독
    mqtt_subscribe(&mqtt_cfg, mqtt_cfg.topic_control);

    // Alive 메시지 발행
    mqtt_publish(&mqtt_cfg, mqtt_cfg.topic_alive, "RP2040 Online");

    printf("=== 메인 루프 시작 ===\n");

    uint32_t last_sensor_send = 0;
    uint32_t last_alive_send = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // MQTT 연결 상태 확인 및 재연결
        if (!mqtt_is_connected(&mqtt_cfg)) {
            printf("⚠️  MQTT 연결 끊김 감지 - 재연결 시도\n");
            // TODO: mqtt_reconnect(&mqtt_cfg, &esp_cfg) 구현 및 호출
            // if (mqtt_reconnect(&mqtt_cfg, &esp_cfg)) {
            //     printf("✅ MQTT 재연결 성공\n");
            //     mqtt_publish(&mqtt_cfg, mqtt_cfg.topic_alive, "reconnected");
            //     last_alive_send = now;
            // } else {
            //     printf("❌ MQTT 재연결 실패 - 5초 후 재시도\n");
            //     sleep_ms(5000);
            //     continue;
            // }
        }

        // 센서 데이터 전송 (10초마다)
        if (now - last_sensor_send > 10000) {
            char sensor_data[64];
            float temperature = 25.5; // TODO: 실제 센서 값 읽기
            float humidity = 60.0;    // TODO: 실제 센서 값 읽기
            snprintf(sensor_data, sizeof(sensor_data), "{\"temp\":%.1f,\"hum\":%.1f}", temperature, humidity);
            printf("센서 데이터 발행: %s\n", sensor_data);
            if (mqtt_publish(&mqtt_cfg, mqtt_cfg.topic_sensor, sensor_data)) {
                last_sensor_send = now;
            } else {
                printf("센서 데이터 발행 실패 - 다음 사이클에 재시도\n");
            }
        }

        // Alive 메시지 전송 (60초마다)
        if (now - last_alive_send > 60000) {
            printf("Alive 메시지 발행\n");
            if (mqtt_publish(&mqtt_cfg, mqtt_cfg.topic_alive, "alive")) {
                last_alive_send = now;
            } else {
                printf("Alive 메시지 발행 실패 - 다음 사이클에 재시도\n");
            }
        }

        // MQTT 메시지 수신 확인 (자주 체크)
        check_mqtt_messages(&mqtt_cfg);

        sleep_ms(50);
    }

    return 0;
}
