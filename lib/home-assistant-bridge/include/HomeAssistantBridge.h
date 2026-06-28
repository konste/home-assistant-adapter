/*!
 * @file
 * @brief
 */

#ifndef HomeAssistantBridge_h
#define HomeAssistantBridge_h

#include <PubSubClient.h>
#include <cstdint>
#include "mqtt_client_adapter.hpp"
#include "tiny_uart_adapter.hpp"

extern "C" {
#include "mqtt_bridge.h"
#include "tiny_gea3_erd_client.h"
#include "tiny_gea3_interface.h"
#include "tiny_timer.h"
#include "uptime_monitor.h"
}

typedef struct {
  const tiny_erd_t* publish_allowlist;
  uint8_t publish_allowlist_count;
  uint32_t min_publish_interval_msec;
  bool publish_only_on_change;
  bool subscribe_to_writes;
  bool publish_write_results;
  uint32_t uptime_publish_interval_msec;
} HomeAssistantBridgeConfiguration;

class HomeAssistantBridge {
 public:
  static constexpr unsigned long baud = 230400;

  void begin(PubSubClient& client, Stream& uart, const char* deviceId, uint8_t clientAddress = 0xE4);
  void begin(
    PubSubClient& client,
    Stream& uart,
    const char* deviceId,
    const HomeAssistantBridgeConfiguration& configuration,
    uint8_t clientAddress = 0xE4);
  void loop();
  void notifyMqttDisconnected();

 private:
  PubSubClient* pubSubClient;

  tiny_timer_group_t timer_group;

  tiny_uart_adapter_t uart_adapter;
  mqtt_client_adapter_t client_adapter;

  tiny_gea3_interface_t gea3_interface;
  uint8_t receive_buffer[255];
  uint8_t send_queue_buffer[1000];

  tiny_gea3_erd_client_t erd_client;
  uint8_t client_queue_buffer[1024];

  mqtt_bridge_t mqtt_bridge;

  uptime_monitor_t uptime_monitor;
};

#endif
