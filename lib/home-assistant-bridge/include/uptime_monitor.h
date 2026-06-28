/*!
 * @file
 * @brief
 *
 * Writes the approximate system uptime to an MQTT topic periodically.
 */

#ifndef uptime_monitor_h
#define uptime_monitor_h

#include "i_mqtt_client.h"
#include "tiny_timer.h"

typedef struct {
  tiny_timer_group_t* timer_group;
  i_mqtt_client_t* mqtt_client;
  tiny_timer_t long_timer;
  tiny_timer_t publish_timer;
  tiny_timer_ticks_t last_remaining_ticks;
  tiny_timer_ticks_t publish_interval_msec;
  uint64_t elapsed_msec;
} uptime_monitor_t;

/*!
 * Initialize an uptime monitor.
 */
void uptime_monitor_init(uptime_monitor_t* self, tiny_timer_group_t* timer_group, i_mqtt_client_t* mqtt_client);
void uptime_monitor_init_with_interval(
  uptime_monitor_t* self,
  tiny_timer_group_t* timer_group,
  i_mqtt_client_t* mqtt_client,
  tiny_timer_ticks_t publish_interval_msec);

#endif
