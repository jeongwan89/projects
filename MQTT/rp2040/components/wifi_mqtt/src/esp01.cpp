#include "esp01.h"
#include "uart_comm.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

void esp01_module_init(Esp01Module& module) {
    printf("[ESP-01] 모듈 초기화 시작\n");
    
    // UART 초기화
    uart_init_esp01(module.uart, module.uart_tx_pin, module.uart_rx_pin, module.uart_baudrate);
    printf("[ESP-01] UART 초기화: TX=%d, RX=%d, Baud=%d\n", 
           module.uart_tx_pin, module.uart_rx_pin, module.uart_baudrate);
    
    // 하드웨어 리셋
    printf("[ESP-01] 하드웨어 리셋 시작 (핀: %d)\n", module.rst_pin);
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
    (void)module;  // 현재는 사용하지 않지만 일관성을 위해 유지
    printf("[ESP-01] AT 명령 초기화 시작\n");
    
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
    uart_wait_response("OK", 2000);
    
    // WiFi 모드 설정 (Station)
    uart_send_at_command("AT+CWMODE=1");
    if (!uart_wait_response("OK", 2000)) {
        printf("[ESP-01] WiFi 모드 설정 실패\n");
        return false;
    }
    
    printf("[ESP-01] AT 명령 초기화 완료\n");
    return true;
}

bool esp01_connect_wifi(Esp01Module& module) {
    printf("[ESP-01] WiFi 연결 시작: %s\n", module.ssid);
    
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", module.ssid, module.password);
    
    for (int i = 0; i < 3; i++) {
        uart_clear_rx_buffer();
        uart_send_at_command(cmd);
        
        if (uart_wait_response("WIFI GOT IP", 15000)) {
            printf("[ESP-01] WiFi 연결 성공\n");
            return true;
        }
        
        printf("[ESP-01] WiFi 연결 실패 (시도 %d/3)\n", i + 1);
        if (i < 2) {
            uart_send_at_command("AT+CWQAP");
            uart_wait_response("OK", 2000);
            sleep_ms(2000);
        }
    }
    
    printf("[ESP-01] WiFi 연결 최종 실패\n");
    return false;
}

bool esp01_is_connected(Esp01Module& module) {
    (void)module;  // 현재는 사용하지 않지만 일관성을 위해 유지
    
    uart_clear_rx_buffer();
    uart_send_at_command("AT+CIPSTATUS");
    
    if (uart_wait_response("STATUS:", 3000)) {
        const char* resp = uart_get_rx_buffer();
        // STATUS:2 = Got IP, STATUS:3 = Connected
        if (strstr(resp, "STATUS:2") || strstr(resp, "STATUS:3")) {
            return true;
        }
    }
    return false;
}
