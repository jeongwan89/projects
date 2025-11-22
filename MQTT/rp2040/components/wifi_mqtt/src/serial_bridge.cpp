#include "serial_bridge.h"
#include <stdio.h>
#include "pico/stdlib.h"

void serial_bridge_mode(uart_inst_t* uart) {
    printf("\n=== 시리얼 브리지 모드 ===\n");
    printf("PC ↔ ESP-01 직접 통신 시작\n");
    printf("Ctrl+C로 종료하세요\n\n");
    
    while (true) {
        // PC(USB) → ESP-01(UART)
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT) {
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
