#ifndef ESP01_ADAPTER_H
#define ESP01_ADAPTER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize ESP01 module (UART + reset) and connect Wi-Fi.
bool esp01_init_and_connect_wifi(unsigned int rst_gpio,
                                 const char *ssid,
                                 const char *password);

#ifdef __cplusplus
}
#endif

#endif // ESP01_ADAPTER_H
