#include <stdio.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

extern "C" {
#include "pico_mqtt.h"
}

const char* SENSOR_LOCATION = "/home/";

/*
//#include "tusb.h"
void wait_for_usb() {
    while (!tud_cdc_connected()) {
        std::cout << ".";
        sleep_ms(500);
    }
    std::cout << "usb host detected\n";
}*/

int main() {
    // setup
    stdio_init_all();
    // if you want to debug over usb, include the function wait_for_usb and define
    // pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
    // in CMakeLists.txt
    //wait_for_usb();

    if (cyw43_arch_init()) {
        std::cout << "Failed to initialize" << std::endl;
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    // Set up our UART with a basic baud rate.
    uart_init(uart0, 115200);
    gpio_set_function(17, GPIO_FUNC_UART);
    gpio_set_function(16, GPIO_FUNC_UART);

    std::cout << "Hello World!" << std::endl;
    std::cout << "Connecting to WiFi..." << std::endl;
    if (cyw43_arch_wifi_connect_timeout_ms(SSID, PSK, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        std::cout << "failed to  connect." << std::endl;
        return 1;
    } else {
        std::cout << "Connected." << std::endl;
    }

    // new MQTT client
    MQTT_CLIENT_T *state = mqtt_client_init(); 
    run_dns_lookup(state);
    mqtt_connect_and_wait(state);

    while (1) {
        mqtt_publish_prepare(state);
        std::string global_topic = std::string(SENSOR_LOCATION);
        
        std::string topic = global_topic + "test";
        std::string value = "value";
        err_t err = mqtt_publish_value(state, topic.c_str(), value.c_str());

        if (err != ERR_OK)
            std::cout << "Publish err: " << err << std::endl;
        std::cout << "Successfully published all MQTT messages." << std::endl;

        sleep_ms(1000);
    }

    cyw43_arch_deinit();
    return 0;
}