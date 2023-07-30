#include "pico_mqtt.h"

// Perform initialisation
MQTT_CLIENT_T* mqtt_client_init(void) {
    MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    state->receiving = 0;
    state->received = 0;
    return state;
}

void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T*)callback_arg;
    DEBUG_printf("DNS query finished with resolved addr of %s.\n", ip4addr_ntoa(ipaddr));
    state->remote_addr = *ipaddr;
}

void run_dns_lookup(MQTT_CLIENT_T *state) {
#ifdef MQTT_SERVER_HOST
    DEBUG_printf("Running DNS query for %s.\n", MQTT_SERVER_HOST);

    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state);
    cyw43_arch_lwip_end();

    if (err == ERR_ARG) {
        DEBUG_printf("failed to start DNS query\n");
        return;
    }

    if (err == ERR_OK) {
        DEBUG_printf("no lookup needed");
        return;
    }
#endif
#ifdef MQTT_SERVER_IP
    ipaddr_aton(MQTT_SERVER_IP, &(state->remote_addr));
    DEBUG_printf("IP addr of MQTT server is %s.\n", ip4addr_ntoa(&(state->remote_addr)));
    DEBUG_printf("IP addr as number: %d\n", state->remote_addr.addr); // 192.168.2.28 -> 3232236060
#endif

    while (state->remote_addr.addr == 0) {
        cyw43_arch_poll();
        sleep_ms(1);
    }
}

// mqtt connection status callback
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    //MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg;
    if (status != 0) {
        DEBUG_printf("Error during connection: err %d.\n", status);
    } else {
        DEBUG_printf("MQTT connected.\n");
    }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg;
    DEBUG_printf("MQTT message sent with code: %d\n", err);
    state->receiving = 0;
    state->received++;
}

err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
  char buffer[128];

  #if MQTT_TLS
  #define TLS_STR "TLS"
  #else
  #define TLS_STR ""
  #endif

  sprintf(buffer, "Hello from picow %d %s", state->received, TLS_STR);

  err_t err;
  u8_t qos = 2; /* 0 1 or 2, see MQTT specification */
  u8_t retain = 0;
  cyw43_arch_lwip_begin();
  err = mqtt_publish(state->mqtt_client, "pico_w/test", buffer, strlen(buffer), qos, retain, mqtt_pub_request_cb, state);
  cyw43_arch_lwip_end();
  if(err != ERR_OK) {
    DEBUG_printf("Publish err: %d\n", err);
  }

  return err;
}

err_t mqtt_publish_prepare(MQTT_CLIENT_T *state) {
    err_t err = ERR_OK;
    u32_t notReady = 5000;
    while (true) {
        cyw43_arch_poll();
        sleep_ms(1);
        if (!notReady--) {
            if (mqtt_client_is_connected(state->mqtt_client)) {
                state->receiving = 1;
                break;
            } else {
                printf(".");
            }
            notReady = 5000;
        }
    }

    return err;
}

err_t mqtt_publish_value(MQTT_CLIENT_T *state, const char *topic, const char *value) {
    err_t err = ERR_OK;
    u8_t qos = 2; /* 0, 1 or 2, see MQTT specification */
    u8_t retain = 0;

    // disable interrupts
    cyw43_arch_lwip_begin();
    err = mqtt_publish(state->mqtt_client, topic, value, strlen(value), qos, retain, mqtt_pub_request_cb, state);
    // enable interrupts again
    cyw43_arch_lwip_end();
    if(err != ERR_OK) {
      DEBUG_printf("Publish err: %d\n", err);
    }

    return err;
}

err_t mqtt_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci;
    err_t err;

    memset(&ci, 0, sizeof(ci));

    // TODO: Define these settings globally!
    ci.client_id = CLIENT_ID;
    ci.client_user = CLIENT_USER;
    ci.client_pass = CLIENT_PASS;
    ci.keep_alive = 0;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_retain = 0;
    ci.will_qos = 2; /* 0, 1 or 2, see MQTT specification */

#if MQTT_TLS
    struct altcp_tls_config *tls_config;
  
    #if defined(CRYPTO_CA) && defined(CRYPTO_KEY) && defined(CRYPTO_CERT)
    DEBUG_printf("Setting up TLS with 2wayauth.\n");
    tls_config = altcp_tls_create_config_client_2wayauth(
        (const u8_t *)ca, 1 + strlen((const char *)ca),
        (const u8_t *)key, 1 + strlen((const char *)key),
        (const u8_t *)"", 0,
        
        (const u8_t *)cert, 1 + strlen((const char *)cert)
    );
    #elif defined(CRYPTO_CERT)
    DEBUG_printf("Setting up TLS with cert.\n");
    tls_config = altcp_tls_create_config_client((const u8_t *) cert, 1 + strlen((const char *) cert));
    #endif

    if (tls_config == NULL) {
        DEBUG_printf("Failed to initialize config\n");
        return -1;
    }

    ci.tls_config = tls_config;
#endif

    // arg: pointer to user data
    //err_t mqtt_client_connect(mqtt_client_t *client, const ip_addr_t *ipaddr, u16_t port, mqtt_connection_cb_t cb, void *arg, const struct mqtt_connect_client_info_t *client_info);
    err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);
    
    if (err != ERR_OK) {
        DEBUG_printf("mqtt_connect return %d\n", err);
    }

    return err;
}

void mqtt_run_test(MQTT_CLIENT_T *state) {
    state->mqtt_client = mqtt_client_new();

    u32_t notReady = 5000;

    if (state->mqtt_client == NULL) {
        DEBUG_printf("Failed to create new mqtt client\n");
        return;
    }

    if (mqtt_connect(state) == ERR_OK) {
        while (true) {
            cyw43_arch_poll();
            sleep_ms(1);
            if (!notReady--) {
                if (mqtt_client_is_connected(state->mqtt_client)) {
                    state->receiving = 1;
                    if (mqtt_test_publish(state) == ERR_OK) {
                        DEBUG_printf("published message\n");
                    } // else ringbuffer is full and we need to wait for messages to flush.
                } else {
                    DEBUG_printf(".");
                }

                // MEM_STATS_DISPLAY();
                // MEMP_STATS_DISPLAY(0);
                // MEMP_STATS_DISPLAY(1);

                notReady = 5000;
            }
        }
    }
}

void mqtt_connect_and_wait(MQTT_CLIENT_T *state) {
    state->mqtt_client = mqtt_client_new();

    u32_t notReady = 5000;

    if (state->mqtt_client == NULL) {
        DEBUG_printf("Failed to create new mqtt client\n");
        return;
    }

    if (mqtt_connect(state) == ERR_OK) {
        DEBUG_printf("Client connected to mqtt server.\n");
        while (true) {
            cyw43_arch_poll();
            sleep_ms(1);
            if (!notReady--) {
                if (mqtt_client_is_connected(state->mqtt_client)) {
                    DEBUG_printf("Ready to publish...\n");
                    break;
                } else {
                    DEBUG_printf(".");
                }
                notReady = 5000;
            }
        }
    }
}