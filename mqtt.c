#include <string.h>
#include "mqtt.h"

void mqtt_connect(mqtt_client_t *client) {
    struct mqtt_connect_client_info_t ci;
    err_t err;

    memset(&ci, 0, sizeof(ci));
    ci.client_id = NODE_NAME NODE_NUMBER;
    ci.keep_alive = 100;
        
    ip_addr_t mqtt_broker;
    //IP4_ADDR(&mqtt_broker, 192, 168, 43, 207);
    //IP4_ADDR(&mqtt_broker, 192, 168, 1, 140);
    IP4_ADDR(&mqtt_broker, 192, 168, 43, 73);

    err = mqtt_client_connect(client, &mqtt_broker, MQTT_PORT, mqtt_connect_cb, 0, &ci);

    if (err != ERR_OK) {
        printf("mqtt_connect error %d\n", err);
    }
}

void mqtt_connect_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    err_t err;
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("mqqt_connect_cb: success\n");

        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
        
        err = mqtt_subscribe(client, MQTT_TOPIC_RECEIVE, 0, mqtt_sub_request_cb, arg); 
        //err = mqtt_subscribe(client, MQTT_TOPIC_RECEIVE, 1, mqtt_sub_request_cb, arg);

        if (err != ERR_OK) {
            printf("mqtt_connect_cb: subscribe returns %d\n", err);
        }
    }
    else {
        printf("mqtt_connect_cb: disconnected %d\n", status);

        mqtt_connect(client);
    }
}


//
void mqtt_sub_request_cb(void *arg, err_t result) {
    printf("Subscribe result: %d\n", result);
}

void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Incoming publish at topic %s with total length %u\n", topic, (unsigned int)tot_len);

    //TODO: modify
    if (strcmp(topic, MQTT_TOPIC_RECEIVE) == 0) {
        inpub_id = 1;
    }
    else {
        inpub_id = 0;
    }
    printf("Set inpub_id %d\n", inpub_id);
}

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Incoming publish payload with length %d, flags %u\n", len, (unsigned int)flags);

    char msg[31] = { 0 };
    if (len < 30) {
        snprintf(msg, len+1, "%s", (const char*)data);
    }
    else {
        snprintf(msg, 30, "%s", (const char*)data);
    }

    printf("Payload: %s\n", msg);

    char* ret;
    ret = strstr(msg, "message"); //message key
    if (ret == NULL) {
        return;
    }

    ret = strstr(msg, "red");
    if (ret != NULL) {
        mqtt_led_noise_level = 3;
        return;
    }

    ret = strstr(msg, "yellow");
    if (ret != NULL) {
        mqtt_led_noise_level = 2;
        return;
    }

    ret = strstr(msg, "green");
    if (ret != NULL) {
        mqtt_led_noise_level = 1;
        return;
    }

    mqtt_led_noise_level = 0;
}

void mqtt_pub_request_cb(void *arg, err_t result) {
    if(result != ERR_OK) {
        printf("\nPublish result: %d\n", result);
    }
}