
#include "board.h"

#ifdef HAS_UART

#include <avr/interrupt.h>
#include <avr/io.h>
#include <LUFA/Drivers/Peripheral/Serial.h>
#include "pigator.h"
#include "ringbuffer.h"
#include "ttydata.h"
#include "display.h"
    
#include "led.h"
#include "serial.h"
#include "arch.h"

#define USART HAS_UART

void (*usbinfunc)(void);

void uart_init(unsigned int baudrate) {

  // USART RX/TX 1
  /* PIN3 (TXD0) as output. */
  UART_PORT.DIRSET = PIN3_bm;
  /* PC2 (RXD0) as input. */
  UART_PORT.DIRCLR = PIN2_bm;
  
  mySerial_Init(&USART, baudrate, USART_CHSIZE_8BIT_gc);
  USART_RxdInterruptLevel_Set(&USART, USART_RXCINTLVL_LO_gc);

}

void uart_task(void) {
  input_handle_func(DISPLAY_USB);
  uart_flush();
}

void uart_flush(void) {
  if (TTY_Tx_Buffer.nbytes)
    USART_DreInterruptLevel_Set(&USART, USART_DREINTLVL_LO_gc);
}

ISR(UART_RXC_vect) {
  USART.STATUS = USART_RXCIF_bm;
  rb_put(&TTY_Rx_Buffer, USART.DATA);
}


ISR(UART_DRE_vect) {

  /* Load the next byte from the USART transmit buffer into the USART if transmit buffer space is available 
     or disable interrupt */
  
  if (TTY_Tx_Buffer.nbytes) {
    USART.DATA = rb_get(&TTY_Tx_Buffer);
  } else 
    USART_DreInterruptLevel_Set(&USART, USART_DREINTLVL_OFF_gc);
  
}

#endif
