#ifndef _MQTT_H
#define _MQTT_H

#include "mqtt-msg.h"

#define MQTT_FLAG_CONNECTED          1
#define MQTT_FLAG_READY              2
#define MQTT_FLAG_EXIT               4

#define MQTT_EVENT_TYPE_NONE                  0
#define MQTT_EVENT_TYPE_CONNECTED             1
#define MQTT_EVENT_TYPE_DISCONNECTED          2
#define MQTT_EVENT_TYPE_SUBSCRIBED            3
#define MQTT_EVENT_TYPE_UNSUBSCRIBED          4
#define MQTT_EVENT_TYPE_PUBLISH               5
#define MQTT_EVENT_TYPE_PUBLISHED             6
#define MQTT_EVENT_TYPE_EXITED                7
#define MQTT_EVENT_TYPE_PUBLISH_CONTINUATION  8

typedef struct mqtt_event_data_t
{
  uint8_t type;
  const char* topic;
  const char* data;
  uint16_t topic_length;
  uint16_t data_length;
  uint16_t data_offset;

} mqtt_event_data_t;

void mqtt_task(void);
int mqtt_publish_with_length(const char* topic, const char* data, int data_length, int qos, int retain);
int mqtt_publish(const char* topic, const char* data, int qos, int retain);
int mqtt_subscribe(const char* topic, int qos);

#endif
