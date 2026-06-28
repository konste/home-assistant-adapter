/*!
 * @file
 * @brief
 */

#include "HomeAssistantBridge.h"

extern "C" {
#include "tiny_time_source.h"
}

static const tiny_gea3_erd_client_configuration_t client_configuration = {
  .request_timeout = 250,
  .request_retries = 10
};

void HomeAssistantBridge::begin(PubSubClient& pubSubClient, Stream& uart, const char* deviceId, uint8_t clientAddress)
{
  static const HomeAssistantBridgeConfiguration default_configuration = {
    .publish_allowlist = nullptr,
    .publish_allowlist_count = 0,
    .min_publish_interval_msec = 0,
    .publish_only_on_change = false,
    .subscribe_to_writes = true,
    .publish_write_results = true,
    .uptime_publish_interval_msec = 1000
  };

  begin(pubSubClient, uart, deviceId, default_configuration, clientAddress);
}

void HomeAssistantBridge::begin(
  PubSubClient& pubSubClient,
  Stream& uart,
  const char* deviceId,
  const HomeAssistantBridgeConfiguration& configuration,
  uint8_t clientAddress)
{
  this->pubSubClient = &pubSubClient;

  tiny_timer_group_init(&timer_group, tiny_time_source_init());

  tiny_uart_adapter_init(&uart_adapter, &timer_group, uart);

  const mqtt_client_adapter_configuration_t mqtt_configuration = {
    .publish_allowlist = configuration.publish_allowlist,
    .publish_allowlist_count = configuration.publish_allowlist_count,
    .min_publish_interval_msec = configuration.min_publish_interval_msec,
    .publish_only_on_change = configuration.publish_only_on_change,
    .subscribe_to_writes = configuration.subscribe_to_writes,
    .publish_write_results = configuration.publish_write_results
  };

  mqtt_client_adapter_init_with_configuration(&client_adapter, &pubSubClient, deviceId, &mqtt_configuration);

  uptime_monitor_init_with_interval(
    &uptime_monitor,
    &timer_group,
    &client_adapter.interface,
    configuration.uptime_publish_interval_msec);

  tiny_gea3_interface_init(
    &gea3_interface,
    &uart_adapter.interface,
    clientAddress,
    send_queue_buffer,
    sizeof(send_queue_buffer),
    receive_buffer,
    sizeof(receive_buffer),
    false);

  tiny_gea3_erd_client_init(
    &erd_client,
    &timer_group,
    &gea3_interface.interface,
    client_queue_buffer,
    sizeof(client_queue_buffer),
    &client_configuration);

  mqtt_bridge_init(
    &mqtt_bridge,
    &timer_group,
    &erd_client.interface,
    &client_adapter.interface);
}

void HomeAssistantBridge::loop()
{
  pubSubClient->loop();
  tiny_timer_group_run(&timer_group);
  tiny_gea3_interface_run(&gea3_interface);
}

void HomeAssistantBridge::notifyMqttDisconnected()
{
  mqtt_client_adapter_notify_mqtt_disconnected(&client_adapter);
}
