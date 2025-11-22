#ifndef SERIAL_BRIDGE_H
#define SERIAL_BRIDGE_H

/**
 * @brief 시리얼 브리지 모드 진입
 * @details PC USB ↔ ESP-01 UART 간 양방향 중계
 *          이 함수는 무한 루프로 동작하며 반환되지 않음
 */
// 구조체 기반 UART 브리지
#include "uart_comm.h"
void serial_bridge_mode(const uart_config_t* cfg);

#endif // SERIAL_BRIDGE_H
