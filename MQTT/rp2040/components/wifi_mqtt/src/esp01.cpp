
#include "esp01.h"
#include "uart_comm.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

void esp01_hardware_reset(const esp01_config_t& cfg) {
    printf("=== ESP-01 하드웨어 리셋 시작 ===\n");
    gpio_init(cfg.rst_pin);
    gpio_set_dir(cfg.rst_pin, GPIO_OUT);
    gpio_put(cfg.rst_pin, 0);
    printf("ESP-01 RST 핀을 LOW로 설정 (리셋 상태)\n");
    sleep_ms(500);
    gpio_put(cfg.rst_pin, 1);
    printf("ESP-01 RST 핀을 HIGH로 설정 (리셋 해제)\n");
    printf("ESP-01 부팅 대기 중 (%d ms)...\n", cfg.reset_delay_ms);
    sleep_ms(cfg.reset_delay_ms);
    printf("ESP-01 부팅 완료\n");
}

bool esp01_init(const esp01_config_t& cfg) {
    printf("=== ESP-01 초기화 ===\n");
    printf("AT 명령 테스트 중...\n");
    bool at_ok = false;
    for (int i = 0; i < 3; i++) {
        send_at_command("AT");
        if (wait_for_response("OK", 2000)) {
            at_ok = true;
            break;
        }
        printf("AT 재시도 %d/3\n", i + 1);
        sleep_ms(500);
    }
    if (!at_ok) {
        printf("ESP-01 응답 없음 - 초기화 실패\n");
        return false;
    }
    printf("AT 응답 확인됨\n");
    send_at_command("ATE0");
    wait_for_response("OK", 2000);
    printf("WiFi 모드 설정 중...\n");
    send_at_command("AT+CWMODE=1");
    if (!wait_for_response("OK", 2000)) {
        printf("WiFi 모드 설정 실패\n");
        return false;
    }
    printf("ESP-01 초기화 완료\n");
    return true;
}

bool wifi_connect(const esp01_config_t& cfg) {
    printf("=== WiFi 연결 ===\n");
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", cfg.ssid, cfg.password);
    for (int i = 0; i < 3; i++) {
        printf("WiFi 연결 시도 %d/3...\n", i + 1);
        send_at_command(cmd);
        if (wait_for_response("WIFI GOT IP", 15000)) {
            printf("WiFi 연결 성공\n");
            return true;
        }
        printf("WiFi 연결 실패 (시도 %d/3)\n", i + 1);
        if (i < 2) {
            printf("재시도 전 대기 중...\n");
            sleep_ms(2000);
            send_at_command("AT+CWQAP");
            wait_for_response("OK", 2000);
        }
    }
    printf("WiFi 연결 최종 실패 (3회 시도 완료)\n");
    return false;
}

bool wifi_is_connected(void) {
    send_at_command("AT+CIPSTATUS");
    
    // 응답에서 STATUS:를 확인
    if (wait_for_response("STATUS:", 3000)) {
        const char* resp = get_rx_buffer();
        // STATUS:2 = Got IP, STATUS:3 = Connected
        if (strstr(resp, "STATUS:2") != NULL || strstr(resp, "STATUS:3") != NULL) {
            return true;
        }
    }
    
    return false;
}
