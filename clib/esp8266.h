#ifndef __ESP8266_H_
#define __ESP8266_H_

#include "ringbuffer.h"

void esp8266_init(void);
void esp8266_task(void);
void esp8266_func(char *in);

extern rb_t ESP_Tx_Buffer;
extern rb_t ESP_Rx_Buffer;

#endif
