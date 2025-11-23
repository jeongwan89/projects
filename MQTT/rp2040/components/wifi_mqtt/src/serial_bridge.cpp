#include "serial_bridge.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

void rp2040_software_reset(void) {
    printf("\n[RESET] RP2040 소프트웨어 리셋 실행...\n");
    sleep_ms(100);  // 메시지 출력 대기
    watchdog_reboot(0, 0, 0);
    while(1);  // watchdog 리셋 대기
}

void serial_bridge_mode(uart_inst_t* uart) {
    printf("\n=== 시리얼 브릿지 모드 ===\n");
    printf("PC ↔ ESP-01 직접 통신 시작\n");
    printf("'reset' 입력 시 RP2040 리셋, Ctrl+C로 종료\n\n");
    
    static char cmd_buffer[16];
    static int cmd_pos = 0;
    
    while (true) {
        // PC(USB) → ESP-01(UART)
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
            // 명령어 버퍼에 저장 (개행 문자 전까지)
            if (ch == '\n' || ch == '\r') {
                cmd_buffer[cmd_pos] = '\0';
                
                // "reset" 명령어 확인
                if (cmd_pos > 0 && strcmp(cmd_buffer, "reset") == 0) {
                    rp2040_software_reset();
                }
                
                cmd_pos = 0;  // 버퍼 초기화
            } else if (cmd_pos < (int)sizeof(cmd_buffer) - 1) {
                cmd_buffer[cmd_pos++] = (char)ch;
            }
            
            uart_putc_raw(uart, (char)ch);
        }
        
        // ESP-01(UART) → PC(USB)
        while (uart_is_readable(uart)) {
            char ch_uart = uart_getc(uart);
            putchar_raw(ch_uart);
        }
        
        tight_loop_contents();
    }
}
