
#ifndef ESP01_H
#define ESP01_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int rst_pin;
    int reset_delay_ms;
    const char* ssid;
    const char* password;
} esp01_config_t;

void esp01_hardware_reset(const esp01_config_t& cfg);
bool esp01_init(const esp01_config_t& cfg);
bool wifi_connect(const esp01_config_t& cfg);
bool wifi_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif // ESP01_H
