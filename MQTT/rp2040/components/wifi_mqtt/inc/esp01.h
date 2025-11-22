#ifndef ESP01_H
#define ESP01_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

// ESP-01 모듈 설정 구조체
typedef struct {
    uart_inst_t* uart;          // UART 인스턴스
    unsigned int uart_tx_pin;   // UART TX 핀
    unsigned int uart_rx_pin;   // UART RX 핀
    unsigned int uart_baudrate; // UART 보드레이트
    unsigned int rst_pin;       // 리셋 핀
    char ssid[64];              // WiFi SSID
    char password[64];          // WiFi 비밀번호
} Esp01Module;

// ESP-01 모듈 초기화 (UART + 하드웨어 리셋)
void esp01_module_init(Esp01Module& module);

// ESP-01 AT 명령 초기화
bool esp01_at_init(Esp01Module& module);

// WiFi 연결
bool esp01_connect_wifi(Esp01Module& module);

// WiFi 연결 상태 확인
bool esp01_is_connected(Esp01Module& module);

#ifdef __cplusplus
}
#endif

#endif // ESP01_H
