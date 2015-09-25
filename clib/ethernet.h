#ifndef __ETHERNET_H_
#define __ETHERNET_H_

#include "ringbuffer.h"

void ethernet_init(void);
void ethernet_func(char *in);
void ethernet_task(void);

extern rb_t NET_Tx_Buffer;

void toNETBuffer(uint8_t in);

#endif
