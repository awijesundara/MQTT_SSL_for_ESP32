#include "Arduino.h"
#include <WiFi.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "mqtt_client.h"

#define SECURE_MQTT // Comment this line if you are not using MQTT over SSL

#ifdef SECURE_MQTT
#include "esp_tls.h"

// CA.CRT Certificate here
static const unsigned char DSTroot_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
YOUR CA.CRT Certificate Here
-----END CERTIFICATE-----
)EOF";
#endif // SECURE_MQTT

esp_mqtt_client_config_t mqtt_cfg;
esp_mqtt_client_handle_t client;

const char* WIFI_SSID = "YOUR SSID";
const char* WIFI_PASSWD = "YOUR PASSWORD";

const char* MQTT_HOST = "YOUR HOST IP";
#ifdef SECURE_MQTT
const uint32_t MQTT_PORT = 8883;
#else
const uint32_t MQTT_PORT = 8883;
#endif // SECURE_MQTT
const char* MQTT_USER = "MQTT USERNAME";
const char* MQTT_PASSWD = "MQTT PASSWORD";

static esp_err_t mqtt_event_handler (esp_mqtt_event_handle_t event) {
  if (event->event_id == MQTT_EVENT_CONNECTED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_CONNECTED", event->msg_id, event->event_id);
    esp_mqtt_client_subscribe (client, "test/hello", 0);
    esp_mqtt_client_publish (client, "test/status", "1", 1, 0, false);
  } 
  else if (event->event_id == MQTT_EVENT_DISCONNECTED) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_DISCONNECTED", event->event_id);
    //esp_mqtt_client_reconnect (event->client); //not needed if autoconnect is enabled
  } else  if (event->event_id == MQTT_EVENT_SUBSCRIBED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_SUBSCRIBED", event->msg_id, event->event_id);
  } else  if (event->event_id == MQTT_EVENT_UNSUBSCRIBED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_UNSUBSCRIBED", event->msg_id, event->event_id);
  } else  if (event->event_id == MQTT_EVENT_PUBLISHED) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_PUBLISHED", event->event_id);
  } else  if (event->event_id == MQTT_EVENT_DATA) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_DATA", event->msg_id, event->event_id);
    ESP_LOGI ("TEST", "Topic length %d. Data length %d", event->topic_len, event->data_len);
    ESP_LOGI ("TEST","Incoming data: %.*s %.*s\n", event->topic_len, event->topic, event->data_len, event->data);

  } else  if (event->event_id == MQTT_EVENT_BEFORE_CONNECT) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_BEFORE_CONNECT", event->event_id);
  }
}

void setup () {
  mqtt_cfg.host = MQTT_HOST;
  mqtt_cfg.port = MQTT_PORT;
  mqtt_cfg.username = MQTT_USER;
  mqtt_cfg.password = MQTT_PASSWD;
  mqtt_cfg.keepalive = 15;
#ifdef SECURE_MQTT
  mqtt_cfg.transport = MQTT_TRANSPORT_OVER_SSL;
#else
  mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
#endif // SECURE_MQTT
  mqtt_cfg.event_handle = mqtt_event_handler;
  mqtt_cfg.lwt_topic = "test/status";
  mqtt_cfg.lwt_msg = "0";
  mqtt_cfg.lwt_msg_len = 1;
  
  Serial.begin (115200);

  WiFi.mode (WIFI_MODE_STA);
  WiFi.begin (WIFI_SSID, WIFI_PASSWD);
  while (!WiFi.isConnected ()) {
    Serial.print ('.');
    delay (100);
  }
  Serial.println ();
#ifdef SECURE_MQTT
  esp_err_t err = esp_tls_set_global_ca_store (DSTroot_CA, sizeof (DSTroot_CA));
  ESP_LOGI ("TEST","CA store set. Error = %d %s", err, esp_err_to_name(err));
#endif // SECURE_MQTT
  client = esp_mqtt_client_init (&mqtt_cfg);
  //esp_mqtt_client_register_event (client, ESP_EVENT_ANY_ID, mqtt_event_handler, client); // not implemented in current Arduino core
  err = esp_mqtt_client_start (client);
  ESP_LOGI ("TEST", "Client connect. Error = %d %s", err, esp_err_to_name (err));
}

void loop () {  
  esp_mqtt_client_publish (client, "test/ack", "This_is_an_Acknowledgement", 27, 0, false);
  delay (2000);
}
