#include "debug_log.h"
#include "mqtt_client.h"
#include "config.h"
#include <stdarg.h>
#include <string.h>

#define DEBUG_TOPIC "test/rp2040/debug"
#define DEBUG_BUFFER_SIZE 256

static bool mqtt_logging_enabled = false;
static char log_buffer[DEBUG_BUFFER_SIZE];

void debug_log_init(void) {
    mqtt_logging_enabled = false;
}

void debug_mqtt_enable(bool enable) {
    mqtt_logging_enabled = enable;
}


void debug_printf(const mqtt_client_config_t* cfg, const char* format, ...) {
    va_list args;

    // USB 출력 (항상 또는 조건부)
#if (DEBUG_LOG_MODE == DEBUG_LOG_USB) || (DEBUG_LOG_MODE == DEBUG_LOG_BOTH)
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif

    // MQTT 출력
#if (DEBUG_LOG_MODE == DEBUG_LOG_MQTT) || (DEBUG_LOG_MODE == DEBUG_LOG_BOTH)
    if (mqtt_logging_enabled && mqtt_is_connected(cfg)) {
        va_start(args, format);
        vsnprintf(log_buffer, DEBUG_BUFFER_SIZE, format, args);
        va_end(args);

        // 줄바꿈 제거 (MQTT 메시지에서는 불필요)
        size_t len = strlen(log_buffer);
        if (len > 0 && log_buffer[len-1] == '\n') {
            log_buffer[len-1] = '\0';
        }

        mqtt_publish(cfg, DEBUG_TOPIC, log_buffer);
    }
#endif
}
