#pragma once

#include "hardware/structs/rosc.h"

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/apps/mqtt.h"

#include "lwip/apps/mqtt_priv.h"

//#define DEBUG_printf printf
#define DEBUG_printf

#include "crypto_consts.h"

#if MQTT_TLS
#ifdef CRYPTO_CERT
const char *cert = CRYPTO_CERT;
#endif
#ifdef CRYPTO_CA
const char *ca = CRYPTO_CA;
#endif
#ifdef CRYPTO_KEY
const char *key = CRYPTO_KEY;
#endif
#endif

typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u8_t receiving;
    u32_t received;
    u32_t reconnect;
} MQTT_CLIENT_T;

err_t mqtt_test_connect(MQTT_CLIENT_T *state);

// Perform initialisation
MQTT_CLIENT_T* mqtt_client_init(void);

void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg);

void run_dns_lookup(MQTT_CLIENT_T *state);

// mqtt connection status callback
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

void mqtt_pub_request_cb(void *arg, err_t err);

err_t mqtt_test_publish(MQTT_CLIENT_T *state);

err_t mqtt_publish_prepare(MQTT_CLIENT_T *state);
err_t mqtt_publish_value(MQTT_CLIENT_T *state, const char *topic, const char *value);

err_t mqtt_connect(MQTT_CLIENT_T *state);

void mqtt_run_test(MQTT_CLIENT_T *state);

void mqtt_connect_and_wait(MQTT_CLIENT_T *state);