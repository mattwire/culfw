
#include "board.h"
#ifdef HAS_MQTT

#include "mqtt.h"
#include <avr/wdt.h>
#include "ethernet.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "mqtt-msg.h"
#include <stdio.h>
#include <string.h>
#include <led.h>
#include <avr/pgmspace.h>

#define  MQTT_SOCK 5
//uint8_t  MQTT_destip[4] = {85, 119, 83, 194};
uint8_t  MQTT_destip[4] = {85, 214, 60, 153};
//uint8_t  MQTT_destip[4] = {10, 10, 11, 64};
uint16_t MQTT_destport = 1883;		     

//process_event_t mqtt_event;
uint8_t  mqtt_state = 0;
int mqtt_flags = 0;

typedef struct mqtt_state_t
{
  //  uip_ip6addr_t address;
  uint16_t port;
  int auto_reconnect;
  mqtt_connect_info_t* connect_info;

  //  struct process* calling_process;
  //  struct uip_conn* tcp_connection;

  //  struct psock ps;
  uint8_t* in_buffer;
  uint8_t* out_buffer;
  int in_buffer_length;
  int out_buffer_length;
  uint16_t message_length;
  uint16_t message_length_read;
  mqtt_message_t* outbound_message;
  mqtt_connection_t mqtt_connection;
  uint16_t pending_msg_id;
  int pending_msg_type;

} mqtt_state_t;

mqtt_state_t mqtt_State;

uint8_t in_buffer[150];
uint8_t out_buffer[150];

mqtt_connect_info_t CI = {
  .client_id = "DT",
  .username  = NULL,
  .password  = NULL,
  .will_topic = NULL,
  .will_message = NULL,
  .keepalive = 0,
  .clean_session = 1,
};

void mqtt_init(void) {


  mqtt_State.in_buffer = in_buffer;
  mqtt_State.in_buffer_length = sizeof(in_buffer)-1;
  mqtt_State.out_buffer = out_buffer;
  mqtt_State.out_buffer_length = sizeof(out_buffer)-1;

  mqtt_State.connect_info = &CI;
  
  printf_P(PSTR("%d:MQTT Init\r\n"), MQTT_SOCK);
  socket(MQTT_SOCK, Sn_MR_TCP, MQTT_destport, 0x0);

}

static void deliver_publish(mqtt_state_t* state, uint8_t* message, int length) {
  mqtt_event_data_t event_data;

  event_data.type = MQTT_EVENT_TYPE_PUBLISH;
  
  event_data.topic_length = length;
  event_data.topic = mqtt_get_publish_topic(message, &event_data.topic_length);
  
  event_data.data_length = length;
  event_data.data = mqtt_get_publish_data(message, &event_data.data_length);

  memmove((char*)event_data.data + 1, (char*)event_data.data, event_data.data_length);
  event_data.data += 1;
  ((char*)event_data.topic)[event_data.topic_length] = '\0';
  ((char*)event_data.data)[event_data.data_length] = '\0';

  printf_P(PSTR(">%s // %s\r\n"), event_data.topic, event_data.data);
  //process_post_synch(state->calling_process, mqtt_event, &event_data);
}


void mqtt_process(mqtt_state_t* state) {
  
  switch (mqtt_state) {
  case 1:
    mqtt_flags = 0;
    mqtt_msg_init(&state->mqtt_connection, state->out_buffer, state->out_buffer_length);
    state->outbound_message = mqtt_msg_connect(&state->mqtt_connection, state->connect_info);
    if (!state->outbound_message->length) {
      printf_P(PSTR("%d:MQTT msg_connect failed building\r\n"), MQTT_SOCK);
      close( MQTT_SOCK );
      return;
    }
    mqtt_state++;
    printf_P(PSTR("%d:MQTT Sending connect msg\r\n"), MQTT_SOCK);
    break;

  case 2:
    if (state->message_length) {
      printf_P(PSTR("received %d\r\n"), state->message_length);

      if(mqtt_get_type(state->in_buffer) != MQTT_MSG_TYPE_CONNACK) {
	printf_P(PSTR("%d:MQTT CONN NACK\r\n"), MQTT_SOCK);
	close(MQTT_SOCK);
	return;
      }
      
      printf_P(PSTR("%d:MQTT ACK\r\n"), MQTT_SOCK);
      // Tell the client we're connected
      mqtt_flags |= MQTT_FLAG_CONNECTED;
      //      complete_pending(state, MQTT_EVENT_TYPE_CONNECTED);
      mqtt_state++;
      state->message_length = 0;
    }
    break;

  case 3:
    if (state->message_length) {
      uint8_t msg_type = mqtt_get_type(state->in_buffer);
      uint8_t msg_qos  = mqtt_get_qos(state->in_buffer);
      uint8_t msg_id   = mqtt_get_id(state->in_buffer, state->in_buffer_length);
      printf_P(PSTR("%d bytes T: %d Q: %d ID: %d\r\n"), state->message_length, msg_type, msg_qos, msg_id);
      switch(msg_type) {
      case MQTT_MSG_TYPE_SUBACK:
	//	if(state->pending_msg_type == MQTT_MSG_TYPE_SUBSCRIBE && state->pending_msg_id == msg_id)
	  //          complete_pending(state, MQTT_EVENT_TYPE_SUBSCRIBED);
        break;
      case MQTT_MSG_TYPE_UNSUBACK:
	//        if(state->pending_msg_type == MQTT_MSG_TYPE_UNSUBSCRIBE && state->pending_msg_id == msg_id)
	  //          complete_pending(state, MQTT_EVENT_TYPE_UNSUBSCRIBED);
        break;
      case MQTT_MSG_TYPE_PUBLISH:
        if(msg_qos == 1)
          state->outbound_message = mqtt_msg_puback(&state->mqtt_connection, msg_id);
        else if(msg_qos == 2)
          state->outbound_message = mqtt_msg_pubrec(&state->mqtt_connection, msg_id);

	deliver_publish(state, state->in_buffer, state->message_length_read);
        break;
      case MQTT_MSG_TYPE_PUBACK:
	//        if(state->pending_msg_type == MQTT_MSG_TYPE_PUBLISH && state->pending_msg_id == msg_id)
	//          complete_pending(state, MQTT_EVENT_TYPE_PUBLISHED);
        break;
      case MQTT_MSG_TYPE_PUBREC:
        state->outbound_message = mqtt_msg_pubrel(&state->mqtt_connection, msg_id);
        break;
      case MQTT_MSG_TYPE_PUBREL:
        state->outbound_message = mqtt_msg_pubcomp(&state->mqtt_connection, msg_id);
        break;
      case MQTT_MSG_TYPE_PUBCOMP:
	//        if(state->pending_msg_type == MQTT_MSG_TYPE_PUBLISH && state->pending_msg_id == msg_id)
	//          complete_pending(state, MQTT_EVENT_TYPE_PUBLISHED);
        break;
      case MQTT_MSG_TYPE_PINGREQ:
        state->outbound_message = mqtt_msg_pingresp(&state->mqtt_connection);
        break;
      case MQTT_MSG_TYPE_PINGRESP:
        // Ignore
        break;
      }
      state->message_length = 0;
    }
    break;

  default:
    break;
  }

}

void mqtt_task(void) {
  int ret;
  uint8_t size = 0;

  switch(getSn_SR(MQTT_SOCK)) {
  case SOCK_ESTABLISHED :
    if(!mqtt_state){
      printf_P(PSTR("%d:MQTT Connected\r\n"), MQTT_SOCK);
      mqtt_state = 1;
    }
    
    // Receive data
    if((mqtt_State.message_length_read = getSn_RX_RSR(MQTT_SOCK)) > 0) {
      size = (mqtt_State.message_length_read>mqtt_State.in_buffer_length) ? mqtt_State.in_buffer_length : mqtt_State.message_length_read;
      ret = recv(MQTT_SOCK, mqtt_State.in_buffer, size);
      if(ret <= 0) {
	close( MQTT_SOCK );
	return;
      }
      mqtt_State.message_length = mqtt_get_total_length(mqtt_State.in_buffer, ret);
    }

    mqtt_process(&mqtt_State);

    // Send out
    if (mqtt_State.outbound_message->length) {
      
      ret = send(MQTT_SOCK, mqtt_State.outbound_message->data, mqtt_State.outbound_message->length);
      if(ret < 0) {
	close(MQTT_SOCK);
	return;
      } else if (ret == mqtt_State.outbound_message->length) { // all bytes gone?
	mqtt_State.outbound_message = NULL;
      } else {
	mqtt_State.outbound_message->length -= ret;
	memcpy( mqtt_State.outbound_message->data, mqtt_State.outbound_message->data+ret, mqtt_State.outbound_message->length);
      }
      
    }
    
    break;
  case SOCK_CLOSE_WAIT :
    printf_P(PSTR("%d:CloseWait\r\n"),MQTT_SOCK);
    disconnect(MQTT_SOCK);
    break;
  case SOCK_CLOSED :
    if (mqtt_state) {
      printf_P(PSTR("%d:Closed\r\n"),MQTT_SOCK);
    }
    mqtt_state = 0;
    // restart - stay connected?
    break;
  case SOCK_INIT :
    mqtt_state = 0;
    printf_P(PSTR("%d:Opened\r\n"),MQTT_SOCK);
    //    uint8_t x = SOCK_IO_NONBLOCK;
    //    ctlsocket(MQTT_SOCK,CS_SET_IOMODE,&x);  
    if((connect(MQTT_SOCK, MQTT_destip, MQTT_destport)) < 0){
      printf_P(PSTR("%d:Connect error\r\n"),MQTT_SOCK);
      close(MQTT_SOCK);
      return;
    }
    printf_P(PSTR("%d:Connecting...\r\n"),MQTT_SOCK);
    //    while (getSn_SR(MQTT_SOCK) == SOCK_INIT) wdt_reset();
    //    x = SOCK_IO_BLOCK;
    //    ctlsocket(MQTT_SOCK,CS_SET_IOMODE,&x);  
    break;
  default :
    printf_P(PSTR("%d:State: %d\r\n"),MQTT_SOCK,getSn_SR(MQTT_SOCK));
    
    break;
  }
  
}

// Publish the specified message
int mqtt_publish_with_length(const char* topic, const char* data, int data_length, int qos, int retain) {

  printf_P(PSTR("mqtt: sending publish...\r\n"));
  mqtt_State.outbound_message = mqtt_msg_publish(&mqtt_State.mqtt_connection, 
                                                 topic, data, data_length, 
                                                 qos, retain,
                                                 &mqtt_State.pending_msg_id);

  if (!mqtt_State.outbound_message->length) 
    printf_P(PSTR("%d:MQTT msg failed to building\r\n"), MQTT_SOCK);
    
  return 0;
}

int mqtt_publish(const char* topic, const char* data, int qos, int retain) {
  return mqtt_publish_with_length(topic, data, strlen(data), qos, retain);
}

int mqtt_subscribe(const char* topic, int qos) {
  printf_P(PSTR("mqtt: sending subscribe for %s ...\r\n"), topic);
  mqtt_State.outbound_message = mqtt_msg_subscribe(&mqtt_State.mqtt_connection, topic, qos, &mqtt_State.pending_msg_id);
  if (!mqtt_State.outbound_message->length)
      printf_P(PSTR("%d:MQTT msg failed to building\r\n"), MQTT_SOCK);
}

void mqtt_func(char *in) {
}

#endif
