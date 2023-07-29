#pragma once

// To use, copy to crypto_consts.h and define either
// CRYPTO_AWS_IOT or CRYPTO_MOSQUITTO_TEST and populate desired values.
//#define MQTT_TLS 1 // needs to be 1 for AWS IoT
//#define CRYPTO_AWS_IOT
#define CRYPTO_MOSQUITTO_TEST

// AWS IoT Core
#ifdef CRYPTO_AWS_IOT
#define MQTT_SERVER_HOST "example.iot.us-east-1.amazonaws.com"
#define MQTT_SERVER_PORT 8883
#define CRYPTO_CERT \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n";
#define CRYPTO_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"-----END RSA PRIVATE KEY-----";
#define CRYPTO_CA \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----";
#endif

// Mosquitto test servers
#ifdef CRYPTO_MOSQUITTO_TEST
#if MQTT_TLS
#define MQTT_SERVER_PORT 8883
#else
#define MQTT_SERVER_PORT 1883
#endif
// use either one of the following two lines:
//#define MQTT_SERVER_HOST "test.mosquitto.org"
#define MQTT_SERVER_IP "192.168.2.28"
#define CRYPTO_CERT \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n"
#endif

// secrets for wifi access
#define SSID "your-ssid"
#define PSK "your-psk"

// secrets for mqtt server access
#define CLIENT_ID "the-name-of-this-device"
#define CLIENT_USER "username-for-authentication"
#define CLIENT_PASS "password-for-authentication"