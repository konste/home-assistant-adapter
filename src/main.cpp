#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "Config.h"
#include "HomeAssistantBridge.h"

#ifndef GE_BRIDGE_PUBLISH_ONLY_ON_CHANGE
#define GE_BRIDGE_PUBLISH_ONLY_ON_CHANGE false
#endif

#ifndef GE_BRIDGE_MIN_PUBLISH_INTERVAL_MSEC
#define GE_BRIDGE_MIN_PUBLISH_INTERVAL_MSEC 0UL
#endif

#ifndef GE_BRIDGE_UPTIME_PUBLISH_INTERVAL_MSEC
#define GE_BRIDGE_UPTIME_PUBLISH_INTERVAL_MSEC 1000UL
#endif

#ifndef GE_BRIDGE_SUBSCRIBE_TO_WRITES
#define GE_BRIDGE_SUBSCRIBE_TO_WRITES true
#endif

#ifndef GE_BRIDGE_PUBLISH_WRITE_RESULTS
#define GE_BRIDGE_PUBLISH_WRITE_RESULTS true
#endif

#ifdef GE_BRIDGE_PUBLISH_ERD_ALLOWLIST
static const tiny_erd_t publishAllowlist[] = GE_BRIDGE_PUBLISH_ERD_ALLOWLIST;
#else
static const tiny_erd_t* publishAllowlist = nullptr;
#endif

#ifdef MQTT_TLS
static WiFiClientSecure wifiClient;
#else
static WiFiClient wifiClient;
#endif
static PubSubClient mqttClient(wifiClient);
static HomeAssistantBridge bridge;
static const HomeAssistantBridgeConfiguration bridgeConfiguration = {
#ifdef GE_BRIDGE_PUBLISH_ERD_ALLOWLIST
  .publish_allowlist = publishAllowlist,
  .publish_allowlist_count = sizeof(publishAllowlist) / sizeof(publishAllowlist[0]),
#else
  .publish_allowlist = nullptr,
  .publish_allowlist_count = 0,
#endif
  .min_publish_interval_msec = GE_BRIDGE_MIN_PUBLISH_INTERVAL_MSEC,
  .publish_only_on_change = GE_BRIDGE_PUBLISH_ONLY_ON_CHANGE,
  .subscribe_to_writes = GE_BRIDGE_SUBSCRIBE_TO_WRITES,
  .publish_write_results = GE_BRIDGE_PUBLISH_WRITE_RESULTS,
  .uptime_publish_interval_msec = GE_BRIDGE_UPTIME_PUBLISH_INTERVAL_MSEC
};

static void connectToWifi()
{
  if(WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Connecting to WiFi...");

  unsigned retries = 0;
  while(WiFi.status() != WL_CONNECTED) {
    if(retries++ > 100) {
      Serial.println("WiFi connection failed, restarting...");
      ESP.restart();
    }

    digitalWrite(LED_WIFI, LOW);
    delay(100);
    Serial.print(".");
  }
}

static void configureWifi()
{
  Serial.println("WiFi SSID: " + String(ssid));

  WiFi.begin(ssid, password);

  connectToWifi();

#ifdef MQTT_TLS
#ifdef MQTT_TLS_VERIFY
  X509List* cert = new X509List(CERT);
  wifiClient.setTrustAnchors(cert);
#else
  wifiClient.setInsecure();
#endif
#endif

  Serial.println("WiFi connected");
}

static void configureMqtt()
{
  mqttClient.setServer(mqtt_server, mqtt_server_port);
}

static void connectToMqtt()
{
  connectToWifi();
  digitalWrite(LED_WIFI, HIGH);

  if(!mqttClient.connected()) {
    digitalWrite(LED_MQTT, LOW);

    unsigned retries = 0;
    while(!mqttClient.connected()) {
      if(retries++ > 10) {
        Serial.println("MQTT connection failed, restarting...");
        ESP.restart();
      }

      Serial.print("Attempting MQTT connection...");

      if(mqttClient.connect("", mqttUser, mqttPassword)) {
        Serial.println("connected");
        digitalWrite(LED_MQTT, HIGH);
      }
      else {
        Serial.println("failed, rc=" + String(mqttClient.state()) + " will try again in 1 second");
        delay(1000);
      }
    }

    bridge.notifyMqttDisconnected();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_HEARTBEAT, OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_MQTT, OUTPUT);

  configureWifi();
  configureMqtt();

  Serial1.begin(HomeAssistantBridge::baud, SERIAL_8N1, D7, D6);
  bridge.begin(mqttClient, Serial1, deviceId, bridgeConfiguration);
}

void loop()
{
  connectToMqtt();
  bridge.loop();
  digitalWrite(LED_HEARTBEAT, millis() % 1000 < 500);
}
