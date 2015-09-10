
#include "board.h"

#ifdef HAS_UART

#include <avr/interrupt.h>
#include <avr/io.h>
#include "ringbuffer.h"
#include "ttydata.h"
#include "display.h"
    
#include "led.h"
#include "serial.h"

/** @brief  UART Baudrate Expression
 *  @param  xtalcpu  system clock in Mhz, e.g. 4000000L for 4Mhz          
 *  @param  baudrate baudrate in bps, e.g. 1200, 2400, 9600     
 */
#define UART_BAUD_SELECT(baudRate,xtalCpu) ((xtalCpu)/((baudRate)*16l)-1)

/** @brief  UART Baudrate Expression for ATmega double speed mode
 *  @param  xtalcpu  system clock in Mhz, e.g. 4000000L for 4Mhz           
 *  @param  baudrate baudrate in bps, e.g. 1200, 2400, 9600     
 */
#define UART_BAUD_SELECT_DOUBLE_SPEED(baudRate,xtalCpu) (((xtalCpu)/((baudRate)*8l)-1)|0x8000)

void (*usbinfunc)(void);

// TX complete (data buffer empty) 
ISR(USART_UDRE_vect)
{
     if (TTY_Tx_Buffer.nbytes) {
	  
	  UDR0 = rb_get(&TTY_Tx_Buffer);
	  
     } else {
	  
	  UCSR0B &= ~_BV(UDRIE0);
	  
     }
     
}

// RX complete
ISR(USART_RX_vect)
{

     LED_TOGGLE();

     /* read UART status register and UART data register */ 
     uint8_t data = UDR0;
     uint8_t usr  = UCSR0A;
     
     if ((usr & (_BV(FE0)|_BV(DOR0))) == 0)
	  rb_put(&TTY_Rx_Buffer, data);
     
}

void uart_init(unsigned int baudrate) {

  baudrate = UART_BAUD_SELECT_DOUBLE_SPEED(baudrate,F_CPU);
  
  /* Set baud rate */
  if ( baudrate & 0x8000 ) {
    UCSR0A = (1<<U2X0);  //Enable 2x speed 
    baudrate &= ~0x8000;
  }
  
  UBRR0 = baudrate;
     
  /* Enable USART receiver and transmitter and receive complete interrupt */
  UCSR0B = _BV(RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
    
  /* Set frame format: asynchronous, 8data, no parity, 1stop bit */   
  UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
     
}

void uart_task(void) 
{
     input_handle_func(DISPLAY_USB);
     uart_flush();
}

void uart_flush(void) 
{
     if (!bit_is_set(UCSR0B, UDRIE0) && TTY_Tx_Buffer.nbytes)
	  UCSR0B |= _BV(UDRIE0);
     
}

#endif
