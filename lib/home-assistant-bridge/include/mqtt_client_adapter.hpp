/*!
 * @file
 * @brief
 */

#ifndef mqtt_client_adapter_hpp
#define mqtt_client_adapter_hpp

#include <PubSubClient.h>
#include <cstdint>
#include <map>
#include <vector>

extern "C" {
#include "i_mqtt_client.h"
#include "tiny_event.h"
#include "tiny_timer.h"
}

typedef struct {
  const tiny_erd_t* publish_allowlist;
  uint8_t publish_allowlist_count;
  uint32_t min_publish_interval_msec;
  bool publish_only_on_change;
  bool subscribe_to_writes;
  bool publish_write_results;
} mqtt_client_adapter_configuration_t;

typedef struct {
  i_mqtt_client_t interface;

  PubSubClient* client;
  const char* device_id;
  tiny_event_t write_request;
  tiny_event_t mqtt_disconnect;
  mqtt_client_adapter_configuration_t configuration;
  std::map<tiny_erd_t, String> last_published_payloads;
  std::map<tiny_erd_t, uint32_t> last_publish_msec;
} mqtt_client_adapter_t;

/*!
 * Initialize an adapter to expose an Arduino PubSubClient as an i_mqtt_client_t.
 */
void mqtt_client_adapter_init(mqtt_client_adapter_t* self, PubSubClient* client, const char* deviceId);
void mqtt_client_adapter_init_with_configuration(
  mqtt_client_adapter_t* self,
  PubSubClient* client,
  const char* deviceId,
  const mqtt_client_adapter_configuration_t* configuration);

/*!
 * Notify the adapter that the MQTT client has connected. Should be called before reconnecting when connection is lost.
 */
void mqtt_client_adapter_notify_mqtt_disconnected(mqtt_client_adapter_t* self);

#endif
