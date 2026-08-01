// ESP32 MQTT-over-TLS example using the ESP-IDF MQTT client that ships
// with the ESP32 Arduino core (esp-idf mqtt_client component), rather
// than PubSubClient + WiFiClientSecure.
//
// Modernized for ESP32 Arduino core 3.x (ESP-IDF 5.x):
//   - esp_mqtt_client_config_t is now a nested struct (broker/credentials/
//     session) instead of the flat struct used pre-IDF5.
//   - Event handling now goes through esp_mqtt_client_register_event()
//     with an esp_event_handler_t signature; the old
//     mqtt_cfg.event_handle callback field was removed.
//   - Credentials and the CA certificate live in secrets.h (git-ignored)
//     instead of being hardcoded here. Copy secrets.example.h to
//     secrets.h and fill in your values before building.
//
// Runtime behavior (topics, QoS, keepalive, LWT, publish cadence) is
// unchanged from the original sketch.

#include "Arduino.h"
#include <WiFi.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include "secrets.h"

#define SECURE_MQTT // Comment this line out to connect over plain TCP instead of TLS.

static const char *TAG = "ESP32_SSL_MCU";

static const char *TOPIC_SUBSCRIBE = "test/hello";
static const char *TOPIC_STATUS    = "test/status";
static const char *TOPIC_ACK       = "test/ack";
static const char *ACK_PAYLOAD     = "This_is_an_Acknowledgement";

static esp_mqtt_client_handle_t client = nullptr;

static void mqtt_event_handler (void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);

  switch (event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI (TAG, "MQTT_EVENT_CONNECTED, msg_id=%d", event->msg_id);
      esp_mqtt_client_subscribe (client, TOPIC_SUBSCRIBE, 0);
      esp_mqtt_client_publish (client, TOPIC_STATUS, "1", 1, 0, false);
      break;
    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI (TAG, "MQTT_EVENT_DISCONNECTED");
      // Not needed: the client auto-reconnects unless disabled in config.
      break;
    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI (TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI (TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI (TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
      break;
    case MQTT_EVENT_DATA:
      ESP_LOGI (TAG, "MQTT_EVENT_DATA, topic_len=%d data_len=%d", event->topic_len, event->data_len);
      ESP_LOGI (TAG, "Incoming: %.*s = %.*s", event->topic_len, event->topic, event->data_len, event->data);
      break;
    case MQTT_EVENT_BEFORE_CONNECT:
      ESP_LOGI (TAG, "MQTT_EVENT_BEFORE_CONNECT");
      break;
    case MQTT_EVENT_ERROR:
      ESP_LOGE (TAG, "MQTT_EVENT_ERROR, type=%d", event->error_handle->error_type);
      break;
    default:
      ESP_LOGI (TAG, "Unhandled MQTT event id=%ld", (long) event_id);
      break;
  }
}

static void connect_wifi () {
  WiFi.mode (WIFI_STA);
  WiFi.begin (WIFI_SSID, WIFI_PASSWORD);
  while (!WiFi.isConnected ()) {
    Serial.print ('.');
    delay (100);
  }
  Serial.println ();
  ESP_LOGI (TAG, "WiFi connected, IP=%s", WiFi.localIP ().toString ().c_str ());
}

static void start_mqtt () {
  esp_mqtt_client_config_t mqtt_cfg = {};

  mqtt_cfg.broker.address.hostname = MQTT_HOST;
  mqtt_cfg.broker.address.port = MQTT_PORT;
#ifdef SECURE_MQTT
  mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_SSL;
  mqtt_cfg.broker.verification.certificate = MQTT_CA_CERT;
  // Alternative: trust the public CA bundle instead of a pinned CA cert.
  //   mqtt_cfg.broker.verification.crt_bundle_attach = esp_crt_bundle_attach;
#else
  mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
#endif // SECURE_MQTT

  mqtt_cfg.credentials.username = MQTT_USERNAME;
  mqtt_cfg.credentials.authentication.password = MQTT_PASSWORD;

  mqtt_cfg.session.keepalive = 15;
  mqtt_cfg.session.last_will.topic = TOPIC_STATUS;
  mqtt_cfg.session.last_will.msg = "0";
  mqtt_cfg.session.last_will.msg_len = 1;

  client = esp_mqtt_client_init (&mqtt_cfg);
  esp_mqtt_client_register_event (client, MQTT_EVENT_ANY, mqtt_event_handler, nullptr);

  esp_err_t err = esp_mqtt_client_start (client);
  ESP_LOGI (TAG, "MQTT client start, err=%d (%s)", err, esp_err_to_name (err));
}

void setup () {
  Serial.begin (115200);
  connect_wifi ();
  start_mqtt ();
}

void loop () {
  esp_mqtt_client_publish (client, TOPIC_ACK, ACK_PAYLOAD, strlen (ACK_PAYLOAD), 0, false);
  delay (2000);
}
