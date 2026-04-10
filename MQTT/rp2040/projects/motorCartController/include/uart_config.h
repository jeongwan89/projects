#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#include "hardware/uart.h"
#include "hardware/gpio.h"

/* UART 설정 */
#define UART_ID         uart1           // UART1 사용
#define UART_BAUDRATE   115200          // 보드레이트
#define UART_TX_PIN     4               // TX - GPIO4
#define UART_RX_PIN     5               // RX - GPIO5

/* UART 초기화 함수 */
void uart_init_custom(void);

/* ESP01 통신 함수 (프로토타입) */
void esp01_send_command(const char *cmd);
void esp01_read_response(char *buffer, int max_len);

#endif // UART_CONFIG_H
