#include "serial_bridge.h"
#include "config.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <pico/stdio_usb.h>
#include "hardware/uart.h"

// PC USB <-> ESP-01 UART 브리지
void serial_bridge_mode(const uart_config_t* cfg) {
    printf("[BRIDGE] 시리얼 브리지 모드 진입 (PC <-> ESP-01)\n");
    printf("PC에서 입력한 내용이 ESP-01로 전달되고, ESP-01의 응답이 PC로 출력됩니다.\n");
    printf("Ctrl+C로 종료하세요.\n");

    while (true) {
        // PC(USB) -> ESP-01(UART)
        int ch = getchar_timeout_us(0);
        if (ch != PICO_ERROR_TIMEOUT && ch != '\r' && ch != '\n') {
            uart_putc_raw(cfg->uart_id, (char)ch);
        }
        // ESP-01(UART) -> PC(USB)
        while (uart_is_readable(cfg->uart_id)) {
            char ch_uart = uart_getc(cfg->uart_id);
            putchar_raw(ch_uart);
        }
        tight_loop_contents();
    }
}
