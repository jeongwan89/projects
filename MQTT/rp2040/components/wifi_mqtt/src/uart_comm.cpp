#include "uart_comm.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/sync.h"  // save_and_disable_interrupts, restore_interrupts

#define RX_BUFFER_SIZE 1024  // ESP-01 긴 응답 처리를 위해 복원

// RP2040 GPIO 유효 범위
#define GPIO_MIN 0
#define GPIO_MAX 29

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
    // NULL 포인터 검증
    if (!uart) {
        printf("[UART] NULL UART 인스턴스\n");
        return;
    }
    
    // 핀 번호 검증
    if (tx_pin < GPIO_MIN || tx_pin > GPIO_MAX) {
        printf("[UART] 유효하지 않은 TX 핀: %d\n", tx_pin);
        return;
    }
    
    if (rx_pin < GPIO_MIN || rx_pin > GPIO_MAX) {
        printf("[UART] 유효하지 않은 RX 핀: %d\n", rx_pin);
        return;
    }
    
    // baudrate 검증 (일반적인 범위)
    if (baudrate < 300 || baudrate > 921600) {
        printf("[UART] 유효하지 않은 baudrate: %d\n", baudrate);
        return;
    }
    
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
    if (!g_uart) {
        printf("[UART] UART 초기화되지 않음\n");
        return;
    }
    
    // NULL 포인터 검증
    if (!cmd) {
        printf("[UART] NULL 명령어\n");
        return;
    }
    
    // TX FIFO가 비워질 때까지 대기
    uart_tx_wait_blocking(g_uart);
    
    uart_puts(g_uart, cmd);
    uart_puts(g_uart, "\r\n");
    
    // 전송 완료 대기
    uart_tx_wait_blocking(g_uart);
}

bool uart_wait_response(const char* expected, uint32_t timeout_ms) {
    // NULL 포인터 검증
    if (!expected) {
        printf("[UART] NULL 예상 응답\n");
        return false;
    }
    
    uint32_t start = to_ms_since_boot(get_absolute_time());
    static char temp_buf[RX_BUFFER_SIZE];
    
    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
        // Critical Section: 링버퍼 읽기 (ISR의 rx_head 업데이트와 충돌 방지)
        uint32_t irq_status = save_and_disable_interrupts();
        
        // 버퍼에서 데이터 복사
        int i = 0;
        uint16_t pos = rx_tail;
        while (pos != rx_head && i < RX_BUFFER_SIZE - 1) {
            temp_buf[i++] = rx_buffer[pos];
            pos = (pos + 1) % RX_BUFFER_SIZE;
        }
        temp_buf[i] = '\0';
        
        restore_interrupts(irq_status);
        
        char* found = strstr(temp_buf, expected);
        if (found) {
            // Critical Section: rx_tail 업데이트 보호
            irq_status = save_and_disable_interrupts();
            
            // 찾은 문자열까지 버퍼에서 소비
            int consume_len = (found - temp_buf) + strlen(expected);
            for (int j = 0; j < consume_len && rx_tail != rx_head; j++) {
                rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
            }
            
            restore_interrupts(irq_status);
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
    // Critical Section: ISR과 동시 접근 방지
    uint32_t irq_status = save_and_disable_interrupts();
    rx_tail = rx_head;
    restore_interrupts(irq_status);
}

void uart_send_raw(const char* data, int len) {
    if (!g_uart) {
        printf("[UART] UART 초기화되지 않음\n");
        return;
    }
    
    // NULL 포인터 검증
    if (!data) {
        printf("[UART] NULL 데이터\n");
        return;
    }
    
    // 음수 길이 검증
    if (len < 0) {
        printf("[UART] 유효하지 않은 길이: %d\n", len);
        return;
    }
    
    if (len == 0) {
        return;  // 전송할 데이터 없음
    }
    
    // TX FIFO가 비워질 때까지 대기
    uart_tx_wait_blocking(g_uart);
    
    for (int i = 0; i < len; i++) {
        uart_putc_raw(g_uart, data[i]);
    }
    
    // 전송 완료 대기
    uart_tx_wait_blocking(g_uart);
}

int uart_read_mqtt_message(char* buffer, int max_len) {
    // NULL 포인터 검증
    if (!buffer) {
        printf("[UART] NULL 버퍼\n");
        return 0;
    }
    
    // 길이 검증
    if (max_len <= 0) {
        printf("[UART] 유효하지 않은 버퍼 크기: %d\n", max_len);
        return 0;
    }
    
    // Critical Section: 링버퍼 읽기 (ISR의 rx_head 업데이트와 충돌 방지)
    uint32_t irq_status = save_and_disable_interrupts();
    
    // 링 버퍼에서 직접 읽기 (static 버퍼 의존성 제거)
    char temp_buf[RX_BUFFER_SIZE];
    int buf_len = 0;
    uint16_t pos = rx_tail;
    
    // 링 버퍼에서 temp_buf로 복사
    while (pos != rx_head && buf_len < RX_BUFFER_SIZE - 1) {
        temp_buf[buf_len++] = rx_buffer[pos];
        pos = (pos + 1) % RX_BUFFER_SIZE;
    }
    temp_buf[buf_len] = '\0';
    
    restore_interrupts(irq_status);
    
    const char* mqtt_start = strstr(temp_buf, "+MQTTSUBRECV");
    
    if (mqtt_start) {
        // 메시지 한 줄만 읽기
        const char* line_end = strchr(mqtt_start, '\n');
        int copy_len = line_end ? (line_end - mqtt_start) : strlen(mqtt_start);
        
        if (copy_len >= max_len) {
            copy_len = max_len - 1;
        }
        
        strncpy(buffer, mqtt_start, copy_len);
        buffer[copy_len] = '\0';
        
        // Critical Section: rx_tail 업데이트 보호
        irq_status = save_and_disable_interrupts();
        
        // 읽은 메시지만 제거 (포인터 산술 오류 수정)
        if (line_end) {
            // mqtt_start부터 line_end까지의 문자 개수 계산
            int consumed = (line_end - mqtt_start) + 1;  // '\n' 포함
            
            // temp_buf의 시작부터 mqtt_start까지의 거리도 고려
            int offset_to_mqtt = mqtt_start - temp_buf;
            int total_consumed = offset_to_mqtt + consumed;
            
            // tail을 안전하게 이동
            for (int i = 0; i < total_consumed && rx_tail != rx_head; i++) {
                rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
            }
        } else {
            // 개행이 없으면 MQTTSUBRECV부터 끝까지 소비
            int offset_to_mqtt = mqtt_start - temp_buf;
            for (int i = 0; i < offset_to_mqtt + copy_len && rx_tail != rx_head; i++) {
                rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
            }
        }
        
        restore_interrupts(irq_status);
        
        return copy_len;
    }
    return 0;
}
