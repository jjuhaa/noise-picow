#ifndef MQTT_H_
#define MQTT_H_

#include "lwip/apps/mqtt.h"

#define WIFI_SSID           ""
#define WIFI_PASS           ""
#define NODE_NUMBER         "4"
#define NODE_NAME           "pico"
#define MQTT_TOPIC_SEND     "noise"
#define MQTT_TOPIC_RECEIVE  "led"

static int inpub_id;

extern int mqtt_led_noise_level;

void mqtt_connect(mqtt_client_t *client);
void mqtt_connect_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_sub_request_cb(void *arg, err_t result);
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void mqtt_pub_request_cb(void *arg, err_t result);

#endif