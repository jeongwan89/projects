#include "esp01.h"
#include "uart_comm.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"

void esp01_module_init(Esp01Module& module) {
    printf("[ESP-01] 모듈 초기화 시작\n");
    
    // NULL 포인터 검증
    if (!module.uart) {
        printf("[ESP-01] 오류: NULL UART 인스턴스\n");
        return;
    }
    
    // GPIO 핀 범위 검증 (RP2040: 0-29)
    if (module.rst_pin > 29) {
        printf("[ESP-01] 오류: 유효하지 않은 RST 핀 (%u)\n", module.rst_pin);
        return;
    }
    
    // UART 초기화 (내부에서 핀 및 baudrate 검증)
    uart_init_esp01(module.uart, module.uart_tx_pin, module.uart_rx_pin, module.uart_baudrate);
    printf("[ESP-01] UART 초기화: TX=%u, RX=%u, Baud=%u\n", 
           module.uart_tx_pin, module.uart_rx_pin, module.uart_baudrate);
    
    // 하드웨어 리셋
    printf("[ESP-01] 하드웨어 리셋 시작 (핀: %u)\n", module.rst_pin);
    gpio_init(module.rst_pin);
    gpio_set_dir(module.rst_pin, GPIO_OUT);
    gpio_put(module.rst_pin, 0);
    sleep_ms(500);
    gpio_put(module.rst_pin, 1);
    printf("[ESP-01] 부팅 대기 중...\n");
    sleep_ms(3000);
    printf("[ESP-01] 모듈 초기화 완료\n");
}

bool esp01_at_init(Esp01Module& module) {
    printf("[ESP-01] AT 명령 초기화 시작\n");
    
    // NULL 포인터 검증
    if (!module.uart) {
        printf("[ESP-01] 오류: NULL UART 인스턴스\n");
        return false;
    }
    
    // AT 명령 테스트
    for (int i = 0; i < 3; i++) {
        uart_clear_rx_buffer();
        uart_send_at_command("AT");
        if (uart_wait_response("OK", 2000)) {
            printf("[ESP-01] AT 응답 확인\n");
            break;
        }
        if (i == 2) {
            printf("[ESP-01] AT 응답 없음\n");
            return false;
        }
        sleep_ms(500);
    }
    
    // 에코 끄기
    uart_send_at_command("ATE0");
    if (!uart_wait_response("OK", 2000)) {
        printf("[ESP-01] 경고: 에코 끄기 실패\n");
        // 계속 진행 (에코가 켜져있어도 동작 가능)
    }
    
    // WiFi 모드 설정 (Station)
    uart_send_at_command("AT+CWMODE=1");
    if (!uart_wait_response("OK", 2000)) {
        printf("[ESP-01] WiFi 스테이션 모드 설정 실패(AT+CWMODE=1)\n");
        return false;
    }
    
    printf("[ESP-01] AT 명령 초기화 완료\n");
    return true;
}

bool esp01_connect_wifi(Esp01Module& module) {
    // SSID 출력 시 format string 취약점 방지
    printf("[ESP-01] WiFi 연결 시작: %.*s\n", (int)sizeof(module.ssid), module.ssid);
    
    // NULL 및 길이 검증
    if (!module.ssid[0]) {
        printf("[ESP-01] 오류: SSID가 비어있음\n");
        return false;
    }
    
    // SSID/비밀번호 길이 검증 (AT 명령어 오버헤드 고려)
    size_t ssid_len = strlen(module.ssid);
    size_t pass_len = strlen(module.password);
    if (ssid_len > 32 || pass_len > 63) {
        printf("[ESP-01] 오류: SSID/비밀번호 길이 초과\n");
        return false;
    }
    
    // 특수문자 검증 (" 문자 금지)
    if (strchr(module.ssid, '"') || strchr(module.password, '"')) {
        printf("[ESP-01] 오류: SSID/비밀번호에 \" 문자 포함 불가\n");
        return false;
    }
    
    // 버퍼 오버플로우 방지 (128 → 256) - 루프 밖에서 한번만 생성
    char cmd[256];
    int written = snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", module.ssid, module.password);
    if (written < 0 || written >= (int)sizeof(cmd)) {
        printf("[ESP-01] 오류: 명령어 생성 실패\n");
        return false;
    }
    
    // WiFi 연결 재시도 루프
    
    for (int i = 0; i < 3; i++) {
        uart_clear_rx_buffer();
        uart_send_at_command(cmd);
        
        if (uart_wait_response("WIFI GOT IP", 15000)) {
            printf("[ESP-01] WiFi 연결 성공\n");
            return true;
        }
        
        printf("[ESP-01] WiFi 연결 실패 (시도 %d/3)\n", i + 1);
        if (i < 2) {
            uart_clear_rx_buffer();
            sleep_ms(50);  // 버퍼 안정화 대기
            uart_send_at_command("AT+CWQAP");
            if (!uart_wait_response("OK", 2000)) {
                printf("[ESP-01] 경고: WiFi 연결 해제 실패\n");
            }
            sleep_ms(2000);
        }
    }
    
    printf("[ESP-01] WiFi 연결 최종 실패\n");
    // 완전 최종실패가 되었을 때, esp01 하드웨어 리셋
    esp01_module_init(module);  // 모듈 재초기화

    return false;
}

bool esp01_is_connected(Esp01Module& module) {
    // NULL 포인터 검증
    if (!module.uart) {
        printf("[ESP-01] 오류: NULL UART 인스턴스\n");
        return false;
    }
    
    // Race condition 완화: 버퍼 클리어 후 즉시 전송
    uart_clear_rx_buffer();
    sleep_ms(50);  // 버퍼 안정화 대기
    uart_send_at_command("AT+CIPSTATUS");
    
    if (uart_wait_response("STATUS:", 3000)) {
        const char* resp = uart_get_rx_buffer();
        
        // NULL 포인터 검증
        if (!resp) {
            printf("[ESP-01] 경고: NULL 응답 버퍼\n");
            return false;
        }
        
        // STATUS:2 = Got IP, STATUS:3 = Connected, STATUS:4 = Connecting
        if (strstr(resp, "STATUS:2") || strstr(resp, "STATUS:3") || strstr(resp, "STATUS:4")) {
            return true;
        }
    }
    return false;
}

bool esp01_reconnect_wifi(Esp01Module& module) {
    printf("[ESP-01] WiFi 재연결 시도...\n");
    
    // NULL 포인터 검증
    if (!module.uart) {
        printf("[ESP-01] 오류: NULL UART 인스턴스\n");
        return false;
    }
    
    // 현재 WiFi 연결 끊기
    uart_clear_rx_buffer();
    uart_send_at_command("AT+CWQAP");
    uart_wait_response("OK", 2000);
    sleep_ms(1000);
    
    // WiFi 재연결 시도
    if (esp01_connect_wifi(module)) {
        printf("[ESP-01] WiFi 재연결 성공\n");
        return true;
    }
    
    printf("[ESP-01] WiFi 재연결 실패\n");
    return false;
}
