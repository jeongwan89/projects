
#ifndef UART_COMM_H
#define UART_COMM_H


#include <stdbool.h>
#include <stdint.h>
#include "hardware/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uart_inst_t* uart_id; // ex: uart0, uart1
	int baud_rate;
	int tx_pin;
	int rx_pin;
} uart_config_t;

void clear_isr_buffer(void);
void uart_init_esp01(const uart_config_t& cfg);
void send_at_command(const uart_config_t& cfg, const char* cmd);
bool wait_for_response(const char* expected, uint32_t timeout_ms);
const char* get_rx_buffer(void);
void clear_rx_buffer(void);
int uart_read_data(void);
int uart_read_mqtt_messages(void);
const char* get_mqtt_buffer(void);
void clear_mqtt_buffer(void);

#ifdef __cplusplus
}
#endif

#endif // UART_COMM_H
