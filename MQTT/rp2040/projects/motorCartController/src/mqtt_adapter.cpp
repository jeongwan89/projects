#include "mqtt_adapter.h"

#include <stdio.h>
#include <string.h>

#include "pico/unique_id.h"
#include "mqtt_client.h"

#define MQTT_BROKER_IP      "192.168.0.24"
#define MQTT_BROKER_PORT    1883
#define MQTT_USERNAME       "farmmain"
#define MQTT_PASSWORD       "eerrtt"
#define MQTT_LWT_TOPIC      "Cart/1/Status"
#define MQTT_LWT_MESSAGE    "Off Line"

static MqttClient g_client;
static char g_client_id[64];

static void build_client_id(void)
{
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);

    char hex[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    pico_get_unique_board_id_string(hex, sizeof(hex));

    // rp2040-<16-byte-hex>
    snprintf(g_client_id, sizeof(g_client_id), "rp2040-%s", hex);
}

bool mqtt_init_and_connect_default(void)
{
    build_client_id();

    memset(&g_client, 0, sizeof(g_client));
    g_client.broker = MQTT_BROKER_IP;
    g_client.port = MQTT_BROKER_PORT;
    g_client.client_id = g_client_id;
    g_client.username = MQTT_USERNAME;
    g_client.password = MQTT_PASSWORD;
    g_client.lwt_topic = MQTT_LWT_TOPIC;
    g_client.lwt_message = MQTT_LWT_MESSAGE;
    g_client.connected = false;
    g_client.last_activity = 0;

    printf("[MQTT] Client ID: %s\n", g_client.client_id);
    return mqtt_connect(g_client);
}

bool mqtt_subscribe_motor_topics(void)
{
    static const char *k_topics[] = {
        "Cart/1/Motor1/Dir",
        "Cart/1/Motor1/Speed",
        "Cart/1/Motor2/Dir",
        "Cart/1/Motor2/Speed",
        "Cart/1/Motor3/Dir",
        "Cart/1/Motor3/Speed",
        "Cart/1/Motor4/Dir",
        "Cart/1/Motor4/Speed",
    };

    for (size_t i = 0; i < (sizeof(k_topics) / sizeof(k_topics[0])); i++) {
        if (!mqtt_subscribe(g_client, k_topics[i], 1)) {
            printf("[MQTT] Subscribe failed: %s\n", k_topics[i]);
            return false;
        }
    }

    printf("[MQTT] Motor topics subscribed.\n");
    return true;
}

bool mqtt_poll_message(char *topic, int topic_max_len, char *message, int message_max_len)
{
    return mqtt_check_message(g_client, topic, topic_max_len, message, message_max_len);
}

bool mqtt_publish_text(const char *topic, const char *message, int qos, int retain)
{
    return mqtt_publish(g_client, topic, message, qos, retain);
}

bool mqtt_publish_device_status(const char *status, int retain)
{
    return mqtt_publish(g_client, MQTT_LWT_TOPIC, status, 1, retain);
}

void mqtt_run_maintenance(void)
{
    mqtt_keepalive(g_client);
}
