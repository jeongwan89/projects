#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

// UART 초기화
void uart_init_esp01(uart_inst_t* uart, unsigned int tx_pin, unsigned int rx_pin, unsigned int baudrate);

// AT 명령 전송
void uart_send_at_command(const char* cmd);

// 응답 대기
bool uart_wait_response(const char* expected, uint32_t timeout_ms);

// 수신 버퍼 읽기
const char* uart_get_rx_buffer(void);

// 수신 버퍼 클리어
void uart_clear_rx_buffer(void);

// 원시 데이터 전송 (MQTT raw publish용)
void uart_send_raw(const char* data, int len);

// MQTT 메시지 읽기
int uart_read_mqtt_message(char* buffer, int max_len);

#ifdef __cplusplus
}
#endif

#endif // UART_COMM_H
