/*!
 * @file
 * @brief
 */

#include <inttypes.h>
#include <stdio.h>
#include "uptime_monitor.h"

static void nothing(void* context)
{
  (void)context;
}

static void publish_uptime(uptime_monitor_t* self)
{
  char payload[20];
  snprintf(payload, sizeof(payload), "%" PRIu64, (uint64_t)(self->elapsed_msec / 1000));
  mqtt_client_publish_sub_topic(self->mqtt_client, "uptime", payload);
}

static void update(void* context)
{
  uptime_monitor_t* self = context;
  tiny_timer_ticks_t remaining_ticks = tiny_timer_remaining_ticks(self->timer_group, &self->long_timer);
  tiny_timer_ticks_t elapsed_ticks = self->last_remaining_ticks - remaining_ticks;
  self->elapsed_msec += elapsed_ticks;
  self->last_remaining_ticks = remaining_ticks;

  publish_uptime(self);
}

void uptime_monitor_init(uptime_monitor_t* self, tiny_timer_group_t* timer_group, i_mqtt_client_t* mqtt_client)
{
  uptime_monitor_init_with_interval(self, timer_group, mqtt_client, 1000);
}

void uptime_monitor_init_with_interval(
  uptime_monitor_t* self,
  tiny_timer_group_t* timer_group,
  i_mqtt_client_t* mqtt_client,
  tiny_timer_ticks_t publish_interval_msec)
{
  self->timer_group = timer_group;
  self->mqtt_client = mqtt_client;
  self->last_remaining_ticks = UINT32_MAX;
  self->publish_interval_msec = publish_interval_msec;
  self->elapsed_msec = 0;

  tiny_timer_start_periodic(
    timer_group,
    &self->long_timer,
    UINT32_MAX,
    self,
    nothing);

  tiny_timer_start_periodic(
    timer_group,
    &self->publish_timer,
    self->publish_interval_msec,
    self,
    update);

  publish_uptime(self);
}
