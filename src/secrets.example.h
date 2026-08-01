// Copy this file to secrets.h and fill in your own values.
// secrets.h is git-ignored so real credentials never get committed.
#pragma once

#define WIFI_SSID     "YOUR SSID"
#define WIFI_PASSWORD "YOUR PASSWORD"

#define MQTT_HOST     "YOUR HOST IP OR HOSTNAME"
#define MQTT_PORT     8883
#define MQTT_USERNAME "MQTT USERNAME"
#define MQTT_PASSWORD "MQTT PASSWORD"

// Paste the CA certificate (PEM format) that signed your MQTT broker's
// server certificate. Leave the placeholder body if you plan to use
// esp_crt_bundle_attach() instead (see README).
static const char MQTT_CA_CERT[] = R"EOF(
-----BEGIN CERTIFICATE-----
YOUR CA.CRT CERTIFICATE HERE
-----END CERTIFICATE-----
)EOF";
