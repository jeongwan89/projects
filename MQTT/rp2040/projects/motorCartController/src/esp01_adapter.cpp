#include "esp01_adapter.h"

#include <stdio.h>
#include <string.h>

#include "esp01.h"
#include "uart_config.h"

bool esp01_init_and_connect_wifi(unsigned int rst_gpio,
                                 const char *ssid,
                                 const char *password)
{
    if (ssid == NULL || password == NULL) {
        printf("[WIFI] Invalid credentials pointer.\n");
        return false;
    }

    Esp01Module module = {
        .uart = UART_ID,
        .uart_tx_pin = UART_TX_PIN,
        .uart_rx_pin = UART_RX_PIN,
        .uart_baudrate = UART_BAUDRATE,
        .rst_pin = rst_gpio,
    };

    strncpy(module.ssid, ssid, sizeof(module.ssid) - 1);
    module.ssid[sizeof(module.ssid) - 1] = '\0';

    strncpy(module.password, password, sizeof(module.password) - 1);
    module.password[sizeof(module.password) - 1] = '\0';

    esp01_module_init(module);

    if (!esp01_at_init(module)) {
        printf("[WIFI] ESP01 AT init failed.\n");
        return false;
    }

    if (!esp01_connect_wifi(module)) {
        printf("[WIFI] ESP01 Wi-Fi connect failed.\n");
        return false;
    }

    return true;
}
