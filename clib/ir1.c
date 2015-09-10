/* Copyright DHS-Computertechnik GmbH, 
   Olaf Droegehorn, 2011.
   Released under the GPL Licence, Version 2
*/

#include "board.h"

#ifdef HAS_IR1

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "ir.h"
#include "display.h"
#include "led.h"

#include "irmp.h"
#include "irsnd.h"

uint8_t ir_mode = 0;

void ir_init( void ) {

  // IR_RX is at PD5
  PORTD.DIRCLR   = PIN5_bm;
  PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;

  // E1 is used for 20kHz clock to rx or tx IR data
  TCE1.CNT   = 0;
  TCE1.CTRLA = TC_CLKSEL_EVCH0_gc;  // 1us clock
  TCE1.CTRLB = TC_WGMODE_NORMAL_gc; // Normal operation
  TCE1.PER   = 50; // 20000 ints per sec
  TCE1.INTCTRLA = 0; // off;

  // IR_TX is at PD0
  // TCD0.CCA

  // PORTE.DIR = 0x01; // set as output
  // TCE0: 1KHz Test signal
  // TCE0.CTRLA = 0x01;      // Prescaler: clk/1
  // TCE0.CTRLB = 0x11;      // CCAEN override, Frequency mode
  // TCE0.CCA   = 15999;     // fFRQ = fPER / (2N CCA+1) = 1KHz // 

  irmp_init();  

}

void ir_task( void ) {
  IRMP_DATA irmp_data;
  
  if (!ir_mode)
    return;
  
  if (!irmp_get_data(&irmp_data))
    return;
  
  if ((ir_mode == 2) && (irmp_data.flags & IRMP_FLAG_REPETITION))
    return;
  
  LED_ON();
  DC('I');
  DH(irmp_data.protocol, 2);
  DH(irmp_data.address, 4);
  DH(irmp_data.command, 4);
  DH(irmp_data.flags, 2);
  DNL();
  LED_OFF();
}

void ir_sample( void ) {
}

uint8_t ir_send_data (void) {
  return 0;
}

void ir_func(char *in) {
  
  switch (in[1]) {
  case 'r': // receive
    if (in[2] && in[3]) {
      fromhex(in+2, &ir_mode, 2);
    }
    TCE1.INTCTRLA = (ir_mode>0) ? 1 : 0; // enable interrupts
    DH(ir_mode,2);
    DNL();
    break;
    
  }

}

ISR(TCE1_OVF_vect) {
  irmp_ISR();  // call irmp ISR
  TCE1.INTFLAGS = 0x01; // clear any interrupt flags
}

#endif
