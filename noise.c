#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/cyw43_arch.h"
#include "mqtt.h"
#include "lwipopts.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "ssd1306_i2c.h"
#include "hardware/watchdog.h"

#define USE_OLED

#define USE_LEDS
#define LED_PIN_RED     0
#define LED_PIN_YELLOW  1
#define LED_PIN_GREEN   17

int mqtt_led_noise_level = 0;

int main(){
    
    stdio_init_all();
    if(cyw43_arch_init()){
        printf("WiFi init failed");
        return -1;
    }
    
    //initialize adc
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    //vars for adc
    uint16_t result;
    double adc_max = 0;
    double adc_min = 3.3;
    double adc_vpp;
    double db_val;

    #ifdef USE_OLED
    //initialize i2c
    i2c_init(SSD1306_I2C_BLOCK, SSD1306_I2C_CLK * 1000);
    gpio_set_function(SSD1306_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_I2C_SDA_PIN);
    gpio_pull_up(SSD1306_I2C_SCL_PIN);

    //initialize oled
    SSD1306_init();
    struct render_area frame_area = {
        start_col: 0,
        end_col: SSD1306_WIDTH - 1,
        start_page: 0,
        end_page: SSD1306_NUM_PAGES - 1
    };
    calc_render_area_buflen(&frame_area);
    //zero the display
    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);
    
    char oled_text[SSD1306_WIDTH] = { 0 };
    #endif

    #ifdef USE_LEDS
    //initialize led pins
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_init(LED_PIN_YELLOW);
    gpio_set_dir(LED_PIN_YELLOW, GPIO_OUT);
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    #endif

    //connect to wifi
    cyw43_arch_enable_sta_mode();
    printf("Connecting to WiFi...\n");
    while (1) {
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
            printf("Failed to connect to WiFi\n");
        }
        else {
            break;
        }
    }
    printf("Connected\n");
    

    //create mqtt client
    mqtt_client_t *client = mqtt_client_new();
    if (client != NULL) {
        mqtt_connect(client);
    }
    else {
        printf("mqtt client init failed\n");
        return -1;
    }

    //vars for mqtt
    err_t err;
    char payload_buf[30] = { 0 };

    watchdog_enable(6000, 0);
    
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    uint64_t time_loop_start = to_us_since_boot(get_absolute_time());
    uint64_t time_now = time_loop_start;
    while (1){

        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        result = adc_read();
        double resultv = result*conversion_factor;
        if (resultv > adc_max) {
            adc_max = resultv;
        }
        if (resultv < adc_min) {
            adc_min = resultv;
        }

        time_now = to_us_since_boot(get_absolute_time());
        if (time_now - time_loop_start < 1000000) {
            sleep_us(125); //~8 kHz smaple rate
            continue;
        }

        adc_vpp = adc_max-adc_min;
        db_val = 20*log10(adc_vpp / 0.2);
        db_val = 60 + 2.060897*db_val - 0.01816239*pow(db_val, 2);

        #ifdef USE_OLED
        //write oled
        memset(buf, 0, SSD1306_BUF_LEN);
        WriteString(buf, 0, 0, "decibel");
        WriteString(buf, 8*8, 0, NODE_NAME NODE_NUMBER);
        memset(oled_text, 0, SSD1306_WIDTH);
        snprintf(oled_text, SSD1306_WIDTH, "%.0f", db_val);
        WriteStringSizeMult(buf, 20, 2*8, oled_text, 6);

        render(buf, &frame_area);
        #endif

        //check mqtt (+wifi) connection
        cyw43_arch_lwip_begin();
        if (mqtt_client_is_connected(client) == 0) {
            printf("MQTT client not connected\n");
            if (cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_JOIN) {
                while (1) {
                    //if (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK)) {
                    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 5000)) {
                        printf("Failed to connect to WiFi\n");
                    }
                    else {
                        break;
                    }
                    watchdog_update();
                }
            }
            mqtt_connect(client);
        }
        cyw43_arch_lwip_end();
        
        //form mqtt message
        memset(payload_buf, 0, 30);
        snprintf(payload_buf, 30, "{\"node\":\"%s\",\"dB\":\"%.1f\"}", NODE_NUMBER, db_val);

	    printf("%s, vpp: %.4f\n", payload_buf, adc_vpp);

        //publish mqtt message
        cyw43_arch_lwip_begin();
        err = mqtt_publish(client, MQTT_TOPIC_SEND, payload_buf, strlen(payload_buf), 0, 0, mqtt_pub_request_cb, NULL);
        cyw43_arch_lwip_end();
        if (err != ERR_OK) {
            printf("Publish err: %d\n", err);
        }

        #ifdef USE_LEDS
        //set leds
        switch (mqtt_led_noise_level)
        {
        case 3:
            gpio_put(LED_PIN_RED, 1);
            gpio_put(LED_PIN_YELLOW, 0);
            gpio_put(LED_PIN_GREEN, 0);
            break;
        case 2:
            gpio_put(LED_PIN_RED, 0);
            gpio_put(LED_PIN_YELLOW, 1);
            gpio_put(LED_PIN_GREEN, 0);
            break;
        case 1:
            gpio_put(LED_PIN_RED, 0);
            gpio_put(LED_PIN_YELLOW, 0);
            gpio_put(LED_PIN_GREEN, 1);
            break;
        default:
            gpio_put(LED_PIN_RED, 0);
            gpio_put(LED_PIN_YELLOW, 0);
            gpio_put(LED_PIN_GREEN, 0);
            break;
        }
        #endif

        //reset loop vars
        adc_max = 0;
        adc_min = 3.3;
        time_loop_start = to_us_since_boot(get_absolute_time());
        
        watchdog_update();
    }
    
    return 0;
}
