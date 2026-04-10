#ifndef MQTT_ADAPTER_H
#define MQTT_ADAPTER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Build MqttClient with project defaults and connect to broker.
bool mqtt_init_and_connect_default(void);

// Subscribe motor control topics.
bool mqtt_subscribe_motor_topics(void);

// Poll one MQTT message if available.
bool mqtt_poll_message(char *topic, int topic_max_len, char *message, int message_max_len);

// Publish a text payload.
bool mqtt_publish_text(const char *topic, const char *message, int qos, int retain);

// Publish device status on LWT topic (Cart/1/Status).
bool mqtt_publish_device_status(const char *status, int retain);

// Run periodic keepalive/connection maintenance.
void mqtt_run_maintenance(void);

#ifdef __cplusplus
}
#endif

#endif // MQTT_ADAPTER_H
