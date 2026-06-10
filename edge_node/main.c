#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED_PIN 15 // normal onboard led is gpio 25
#define SOIL_MOISTURE_PIN 26

// some controllers will have this psk will be physically burned into the device
// for pico, i hardcode it here for simplicity
const char* DEVICE_PSK = "HMAC_PresharedSecretKeyPicoOptee"; 

void hmac_sha256(const uint8_t *key, size_t key_len, 
                 const uint8_t *data, size_t data_len, 
                 uint8_t *mac_out);

int main() {
    // init standard i/o (routes printf and getchar over USB Serial)
    stdio_init_all();

    // setup led
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // setup soil moisture adc
    adc_init();
    adc_gpio_init(SOIL_MOISTURE_PIN);
    adc_select_input(0); 

    char buffer[64];
    int buf_idx = 0;
    uint32_t last_telemetry_time = 0;

    while (true) {
        uint32_t current_time = time_us_32();

        // send tele every 2s (nonblocking delay)
        if (current_time - last_telemetry_time > 2000000) {
            uint16_t soil_moisture = adc_read();

            char raw_payload[64];
            snprintf(raw_payload, sizeof(raw_payload), "{\"soil_moisture\": %d}", soil_moisture);

            uint8_t mac_out[32];
            hmac_sha256((const uint8_t*)DEVICE_PSK, strlen(DEVICE_PSK), 
                        (const uint8_t*)raw_payload, strlen(raw_payload), 
                        mac_out);

            // convert 32 byte mac to 64char hex string
            char hex_mac[65] = {0};
            for(int i = 0; i < 32; i++) {
                sprintf(&hex_mac[i * 2], "%02x", mac_out[i]);
            }
            printf("{\"payload\": %s, \"hmac\": \"%s\"}\n", raw_payload, hex_mac);

            last_telemetry_time = current_time;
        }

        // check for incoming commands over serial (nonblocking)
        int c = getchar_timeout_us(0);
        
        if (c != PICO_ERROR_TIMEOUT) {
            // lf or cr to mark end of cmd
            if (c == '\n' || c == '\r') {
                buffer[buf_idx] = '\0'; // terminate str
                
                // parse cmd
                if (strcmp(buffer, "led_on") == 0) {
                    gpio_put(LED_PIN, 1);
                    printf("{\"status\": \"success\", \"led_state\": 1}\n");
                } else if (strcmp(buffer, "led_off") == 0) {
                    gpio_put(LED_PIN, 0);
                    printf("{\"status\": \"success\", \"led_state\": 0}\n");
                } else if (buf_idx > 0) { 
                    printf("{\"status\": \"error\", \"message\": \"unknown command\"}\n");
                }
                
                buf_idx = 0; //reset buffer for next cmd
            } else if (buf_idx < sizeof(buffer) - 1) {
                // add char to buffer
                buffer[buf_idx++] = (char)c;
            }
        }
    }
    return 0;
}