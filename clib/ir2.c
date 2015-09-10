/* Copyright DHS-Computertechnik GmbH, 
   Olaf Droegehorn, 2011.
   Released under the GPL Licence, Version 2
*/

#include "board.h"

#ifdef HAS_IR2

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
//#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"
#include "arch.h"
#include "led.h"
#include "display.h"
#include "xeeprom.h"

#define EE_START 500

uint8_t ir_mode  = 0;
uint8_t ir_learn = 0;
uint8_t ir_state = 0;

typedef struct {
  uint8_t data[64];
  uint8_t pos;
  uint8_t id;
} IR_Data_t;

IR_Data_t ird;
IR_Data_t irl;
IR_Data_t IRCodes[5];

void ir_init(void) {
  ir_state = 0;
  ir_learn = 0;

  while((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
  NVM.CTRLB |= NVM_EEMAPEN_bm;
  
  IRCodes[0].id = 1;

  // IR_RX is at PD5
  PORTD.DIRCLR   = PIN5_bm;
  PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
  PORTD.INT0MASK = PIN5_bm;
  PORTD.INTCTRL  = PORT_INT0LVL_LO_gc;

  TCD0.CNT   = 0;
  TCD0.CTRLA = TC_CLKSEL_EVCH0_gc;
  TCD0.PER   = 15000;
  TCD0.INTCTRLA = 0; // off;
  
  /*
  IRMP_PORT.DIRCLR = _BV(IRMP_BIT); // as input

  // setup interrupt * any edge *
  IRMP_PORT.IRMP_PINCTRL = (PORT_ISC_BOTHEDGES_gc | PORT_OPC_PULLUP_gc);
  IRMP_PORT.INTCTRL |= PORT_INT1LVL_HI_gc;
  IRMP_PORT.IRMP_INTMASK = _BV(IRMP_BIT); // on
  */
}

uint8_t ir_equal(IR_Data_t *A, IR_Data_t *B, uint8_t fuzz) {
  if (!A || !B)
    return 0;

  if (A->pos != B->pos)
    return 0;

  for (uint8_t i=0; i<A->pos; i++) {
    if (abs(A->data[i] - B->data[i]) > (A->data[i] * fuzz / 100))
      return 0;
  }
  
  return 1;
}

uint8_t ir_cmp(IR_Data_t *A, IR_Data_t *B) {
  uint8_t d = 0;
  
  if (!A || !B)
    return 100;

  if (A->pos != B->pos)
    return 100;

  for (uint8_t i=0; i<A->pos; i++) {
    if (!A->data[i])
      continue;

    uint8_t diff = 100 * abs(A->data[i] - B->data[i]) / A->data[i];

    if (diff>d)
      d = diff;
  }
  
  return d;
}

uint8_t ir_avg(IR_Data_t *A, IR_Data_t *B) {
  if (!A || !B)
    return 0;

  if (A->pos != B->pos)
    return 0;

  for (uint8_t i=0; i<A->pos; i++) {
    A->data[i] = (A->id*A->data[i] + B->data[i]) / (A->id+1);
  }

  A->id++;
  
  return 1;
}


void ir_task(void) {
  
  if (!ir_mode)
    return;

  // received new code?
  if (IRCodes[0].id)
    return;

  // mark it "old"
  IRCodes[0].id = 1;

  // Dump it
  // printf( "%d: ", IRCodes[0].pos ); for (uint8_t i=0; i<IRCodes[0].pos; i++) printf( "%04x ", IRCodes[0].data[i]); printf( "\r\n");

  if (ir_learn) {
    
    // first sample?
    if (irl.id) {
      if ((irl.id == 1) && (ir_cmp(&IRCodes[0], &irl)>50)) {
	memcpy( &irl, &IRCodes[0], sizeof(IR_Data_t));
	irl.id = 1;
      } else 
	ir_avg( &irl, &IRCodes[0] );
    } else {
      memcpy( &irl, &IRCodes[0], sizeof(IR_Data_t));
      irl.id = 1;
    }

    printf( "learned: %d - %d%%\r\n", irl.id, 100-ir_cmp(&IRCodes[0], &irl) );

    if (irl.id>9) {
      //      memcpy( &IRCodes[ir_learn], &irl, sizeof(IR_Data_t) );
      uint16_t addr = EE_START + ir_learn * sizeof(IR_Data_t);
      //      eeprom_busy_wait();
      //      memcpy( (void *)addr, &irl, sizeof(IR_Data_t) );
      irl.id = ir_learn;
      eeprom_write_block( &irl, (void*)addr, sizeof(IR_Data_t)); 
      while((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
      ir_learn = 0;
    }
    
    return;
  } 
  
  // look it up
  for (uint8_t i=1; i<=20; i++) {
    uint16_t addr = EE_START + i * sizeof(IR_Data_t);
    //    eeprom_busy_wait();
    //    memcpy(&IRCodes[1],(void *)addr,sizeof(IR_Data_t));
    eeprom_read_block( &IRCodes[1], (void*)addr, sizeof(IR_Data_t)); 
    //    for (uint8_t i=0; i<IRCodes[1].pos; i++) printf( "%02x ", IRCodes[1].data[i]);
    if (ir_equal( &IRCodes[0], &IRCodes[1], 20)) {
      printf( "matched: %d - %d%%\r\n", IRCodes[1].id, 100-ir_cmp(&IRCodes[0], &IRCodes[1]) );
      return;
    }
  }
  
}

void ir_sample(void) {
}

uint8_t ir_send_data(void) {
}

void ir_func(char *in) {
  uint8_t i;
  
  switch (in[1]) {
  case 'r': // receive
    if (in[2] && in[3]) {
      fromhex(in+2, &ir_mode, 2);
    }
    ir_init();
    DH(ir_mode,2);
    DNL();
    break;

  case 's': // store
    if (in[2] && in[3]) {
      fromhex(in+2, &i, 2);
      if (i>0 && i<10) {
	memcpy( &IRCodes[i], &IRCodes[0], sizeof(IR_Data_t) );
	printf( "OK %d\r\n", i );
      }
    }
    break;

  case 'l': // learn
    if (in[2]) {
      in += 2;
      ir_learn = 0;
      while((i = *in++))
	if(i >= '0' && i <= '9')
	  ir_learn = ir_learn*10 + (i-'0');
      memset( &irl, 0, sizeof(ird) );
      printf( "enter learning for: %d\r\n", ir_learn );
    }
    break;
  }
  
}

ISR(TCD0_OVF_vect) {
  // Timeout - we received a packet
  TCD0.INTCTRLA = 0; // off;
  ir_state = 0;

  // if we received sufficent data we keep it at idx 0
  if (ird.pos>20) {
    memcpy( &IRCodes[0], &ird, sizeof(IR_Data_t));
    IRCodes[0].id = 0; // mark it unprocessed
  }
}

ISR(PORTD_INT0_vect) {
  LED_TOGGLE();

  uint16_t cnt = TCD0.CNT / 10;
  TCD0.CNT = 0;

  switch (ir_state) {
  case 0:
    // we start if level is high, so no pulse
    if(!bit_is_set(PORTD.IN,5))
      break;
    TCD0.INTCTRLA = 1; // on;
    ir_state = 1;
    memset( &ird, 0, sizeof(ird) );
    break;
  case 1:
    if (ird.pos<sizeof(ird.data)) {
      ird.data[ird.pos++] = cnt;
      break;
    }
  default:
    TCD0.INTCTRLA = 0; // off;
    ir_state = 0;
  }
}

    
#endif
