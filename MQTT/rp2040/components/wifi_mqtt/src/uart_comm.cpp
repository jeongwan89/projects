
#include "uart_comm.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define RX_BUFFER_SIZE 1024

static volatile char rx_buffer[RX_BUFFER_SIZE];
static volatile uint16_t isr_write_pos = 0;
static volatile uint16_t isr_read_pos = 0;

static char mqtt_buffer[RX_BUFFER_SIZE];
static size_t mqtt_buf_len = 0;

uart_inst_t* g_uart_id = uart1;
static int g_tx_pin = 4;
static int g_rx_pin = 5;

// UART 인터럽트 핸들러
static void on_uart_rx() {
	while (uart_is_readable(g_uart_id)) {
		char ch = uart_getc(g_uart_id);
		uint16_t next_pos = (isr_write_pos + 1);
		if (next_pos >= RX_BUFFER_SIZE) next_pos = 0; // wrap-around
		if (next_pos != isr_read_pos) { // 버퍼 오버플로 방지
			rx_buffer[isr_write_pos] = ch;
			isr_write_pos = next_pos;
		} else {
			// 오버플로우: 가장 오래된 데이터 버림 (read_pos를 한 칸 앞으로)
			isr_read_pos = (isr_read_pos + 1) % RX_BUFFER_SIZE;
			rx_buffer[isr_write_pos] = ch;
			isr_write_pos = next_pos;
		}
	}
}

void uart_init_esp01(const uart_config_t& cfg) {
	g_uart_id = cfg.uart_id;
	g_tx_pin = cfg.tx_pin;
	g_rx_pin = cfg.rx_pin;
	uart_init(g_uart_id, cfg.baud_rate);
	gpio_set_function(g_tx_pin, GPIO_FUNC_UART);
	gpio_set_function(g_rx_pin, GPIO_FUNC_UART);
	uart_set_hw_flow(g_uart_id, false, false);
	uart_set_format(g_uart_id, 8, 1, UART_PARITY_NONE);
	uart_set_fifo_enabled(g_uart_id, true);
	// 인터럽트 등록
	if (g_uart_id == uart1) {
		irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
		irq_set_enabled(UART1_IRQ, true);
	} else if (g_uart_id == uart0) {
		irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
		irq_set_enabled(UART0_IRQ, true);
	}
	uart_set_irq_enables(g_uart_id, true, false);
	clear_isr_buffer();
}

void send_at_command(const char* cmd) {
	uart_puts(g_uart_id, cmd);
	uart_puts(g_uart_id, "\r\n");
}

bool wait_for_response(const char* expected, uint32_t timeout_ms) {
	uint32_t start = to_ms_since_boot(get_absolute_time());
	static char temp_buf[RX_BUFFER_SIZE];
	while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
		int len = uart_read_data();
		if (len > 0) {
			// 버퍼 복사(널 종료)
			int i = 0, j = isr_read_pos;
			while (j != isr_write_pos && i < RX_BUFFER_SIZE - 1) {
				temp_buf[i++] = rx_buffer[j];
				j = (j + 1) % RX_BUFFER_SIZE;
			}
			temp_buf[i] = '\0';
			if (strstr(temp_buf, expected)) {
				return true;
			}
		}
		sleep_ms(10);
	}
	return false;
}

const char* get_rx_buffer(void) {
	static char out_buf[RX_BUFFER_SIZE];
	int i = 0, j = isr_read_pos;
	while (j != isr_write_pos && i < RX_BUFFER_SIZE - 1) {
		out_buf[i++] = rx_buffer[j];
		j = (j + 1) % RX_BUFFER_SIZE;
	}
	out_buf[i] = '\0';
	return out_buf;
}

void clear_rx_buffer(void) {
	isr_read_pos = isr_write_pos;
}

int uart_read_data(void) {
	// 버퍼에 쌓인 데이터 개수 반환
	int count = 0;
	uint16_t r = isr_read_pos;
	while (r != isr_write_pos) {
		r = (r + 1) % RX_BUFFER_SIZE;
		count++;
	}
	return count;
}

// MQTT 메시지용 별도 버퍼 관리 (구독 메시지 수신)
int uart_read_mqtt_messages(void) {
	// RX 버퍼에서 +MQTTSUBRECV로 시작하는 라인만 mqtt_buffer에 복사
	int copied = 0;
	int i = 0, j = isr_read_pos;
	mqtt_buf_len = 0;
	while (j != isr_write_pos && i < RX_BUFFER_SIZE - 1) {
		char ch = rx_buffer[j];
		if (ch == '+') {
			// +MQTTSUBRECV로 시작하는지 확인
			int k = 0;
			char temp[16] = {0};
			int jj = j;
			while (k < 15 && jj != isr_write_pos) {
				temp[k++] = rx_buffer[jj];
				jj = (jj + 1) % RX_BUFFER_SIZE;
			}
			temp[k] = '\0';
			if (strstr(temp, "+MQTTSUBRECV") == temp) {
				// 한 줄 복사
				int l = 0;
				while (j != isr_write_pos && l < RX_BUFFER_SIZE - 1) {
					char c = rx_buffer[j];
					mqtt_buffer[l++] = c;
					if (c == '\n') break;
					j = (j + 1) % RX_BUFFER_SIZE;
				}
				mqtt_buffer[l] = '\0';
				mqtt_buf_len = l;
				copied = l;
				break;
			}
		}
		j = (j + 1) % RX_BUFFER_SIZE;
		i++;
	}
	return copied;
}

const char* get_mqtt_buffer(void) {
	return mqtt_buffer;
}

void clear_mqtt_buffer(void) {
	mqtt_buffer[0] = '\0';
	mqtt_buf_len = 0;
}

void clear_isr_buffer(void) {
	isr_read_pos = 0;
	isr_write_pos = 0;
	memset((void*)rx_buffer, 0, RX_BUFFER_SIZE);
}
