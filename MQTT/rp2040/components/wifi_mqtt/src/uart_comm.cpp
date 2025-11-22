#include "uart_comm.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"

#define RX_BUFFER_SIZE 512

static uart_inst_t* g_uart = NULL;
static volatile char rx_buffer[RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;  // 쓰기 위치 (생산자)
static volatile uint16_t rx_tail = 0;  // 읽기 위치 (소비자)

// UART RX 인터럽트 핸들러
static void on_uart_rx(void) {
    while (uart_is_readable(g_uart)) {
        char ch = uart_getc(g_uart);
        uint16_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;
        if (next_head != rx_tail) {
            rx_buffer[rx_head] = ch;
            rx_head = next_head;
        }
    }
}

void uart_init_esp01(uart_inst_t* uart, unsigned int tx_pin, unsigned int rx_pin, unsigned int baudrate) {
    g_uart = uart;
    
    uart_init(uart, baudrate);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(uart, false, false);
    uart_set_format(uart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart, true);
    
    // 인터럽트 설정
    int uart_irq = (uart == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(uart_irq, on_uart_rx);
    irq_set_enabled(uart_irq, true);
    uart_set_irq_enables(uart, true, false);
    
    rx_head = 0;
    rx_tail = 0;
}

void uart_send_at_command(const char* cmd) {
    if (!g_uart) return;
    
    // TX FIFO가 비워질 때까지 대기
    uart_tx_wait_blocking(g_uart);
    
    uart_puts(g_uart, cmd);
    uart_puts(g_uart, "\r\n");
    
    // 전송 완료 대기
    uart_tx_wait_blocking(g_uart);
}

bool uart_wait_response(const char* expected, uint32_t timeout_ms) {
    uint32_t start = to_ms_since_boot(get_absolute_time());
    static char temp_buf[RX_BUFFER_SIZE];
    
    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
        // 버퍼에서 데이터 복사
        int i = 0;
        uint16_t pos = rx_tail;
        while (pos != rx_head && i < RX_BUFFER_SIZE - 1) {
            temp_buf[i++] = rx_buffer[pos];
            pos = (pos + 1) % RX_BUFFER_SIZE;
        }
        temp_buf[i] = '\0';
        
        if (strstr(temp_buf, expected)) {
            return true;
        }
        sleep_ms(10);
    }
    return false;
}

const char* uart_get_rx_buffer(void) {
    static char out_buf[RX_BUFFER_SIZE];
    int i = 0;
    uint16_t pos = rx_tail;
    
    while (pos != rx_head && i < RX_BUFFER_SIZE - 1) {
        out_buf[i++] = rx_buffer[pos];
        pos = (pos + 1) % RX_BUFFER_SIZE; // 간단하지만 안전한 원형 버퍼 읽기
    }
    out_buf[i] = '\0';
    return out_buf;
}

void uart_clear_rx_buffer(void) {
    rx_tail = rx_head;
}

void uart_send_raw(const char* data, int len) {
    if (!g_uart) return;
    
    // TX FIFO가 비워질 때까지 대기
    uart_tx_wait_blocking(g_uart);
    
    for (int i = 0; i < len; i++) {
        uart_putc_raw(g_uart, data[i]);
    }
    
    // 전송 완료 대기
    uart_tx_wait_blocking(g_uart);
}

int uart_read_mqtt_message(char* buffer, int max_len) {
    const char* rx = uart_get_rx_buffer();
    const char* mqtt_start = strstr(rx, "+MQTTSUBRECV");
    
    if (mqtt_start) {
        int len = 0;
        while (*mqtt_start && *mqtt_start != '\n' && len < max_len - 1) {
            buffer[len++] = *mqtt_start++;
        }
        buffer[len] = '\0';
        uart_clear_rx_buffer();
        return len;
    }
    return 0;
}
