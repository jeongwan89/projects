#ifndef SERIAL_BRIDGE_H
#define SERIAL_BRIDGE_H

#include "hardware/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

// 시리얼 브리지 모드 (PC ↔ ESP-01 직접 통신)
void serial_bridge_mode(uart_inst_t* uart);

#ifdef __cplusplus
}
#endif

#endif // SERIAL_BRIDGE_H
