#ifndef _ARCH_H
#define _ARCH_H

#include "board.h"

/*------------  PLATFORM depended --------------
*/

void cpu_set_clock(void);
void start_bootloader(void);
void check_bootloader_req(void);

void setup_io(void);
void setup_timer(void);

/* SPI */
void spi_init(void);
uint8_t spi_send(uint8_t data);

/* HAL for CC1100 */
uint8_t gdo0_is_set(void);
uint8_t gdo2_is_set(void);
uint8_t is433MHz(void);

/* major delay() i.e. for bit timing */
void my_delay_us( uint16_t d );
void my_delay_ms( uint16_t d );

#ifdef XMEGA

#define BITTC TCD1
#define BITINT TCD1_OVF_vect
#define BITCNT BITTC.CNT
#define BITMAX BITTC.PER
#define BITINT_OFF BITTC.INTCTRLA = 0
#define BITINT_ON  BITTC.INTCTRLA = 1
#define BITINT_CLR BITTC.INTFLAGS |= 1

#define CC1100_OUT_SET CC1100_OUT_PORT.OUTSET = CC1100_OUT_PIN
#define CC1100_OUT_CLR CC1100_OUT_PORT.OUTCLR = CC1100_OUT_PIN
#define PININT CC1100_IN_INT
#define PININT_ON CC1100_IN_PORT.CC1100_IN_INTMASK = CC1100_IN_PIN
#define PININT_OFF CC1100_IN_PORT.CC1100_IN_INTMASK = 0

void mySerial_Init(USART_t* const USART, const uint32_t BaudRate);

#define USART_Baudrate_Set(_usart, _bselValue, _bScaleFactor)                  \
	(_usart)->BAUDCTRLA =(uint8_t)_bselValue;                              \
	(_usart)->BAUDCTRLB =(_bScaleFactor << USART_BSCALE0_bp)|(_bselValue >> 8)

#define USART_RxdInterruptLevel_Set(_usart, _rxdIntLevel)                      \
	((_usart)->CTRLA = ((_usart)->CTRLA & ~USART_RXCINTLVL_gm) | _rxdIntLevel)

#define USART_DreInterruptLevel_Set(_usart, _dreIntLevel)                      \
	(_usart)->CTRLA = ((_usart)->CTRLA & ~USART_DREINTLVL_gm) | _dreIntLevel

#define USART_TxdInterruptLevel_Set(_usart, _txdIntLevel)		\
  ((_usart)->CTRLA = ((_usart)->CTRLA & ~USART_TXCINTLVL_gm) | _txdIntLevel)

#else // XMEGA

#define BITCNT TCNT1
#define BITMAX OCR1A
#define BITINT TIMER1_COMPA_vect
#define BITINT_OFF TIMSK1 = 0
#define BITINT_ON  TIMSK1 = _BV(OCIE1A)
#define BITINT_CLR TIFR1  = _BV(OCF1A)

#define CC1100_OUT_SET CC1100_OUT_PORT |= _BV(CC1100_OUT_PIN)
#define CC1100_OUT_CLR CC1100_OUT_PORT &= ~_BV(CC1100_OUT_PIN)
#define PININT CC1100_INTVECT
#define PININT_ON  EIMSK |= _BV(CC1100_INT)
#define PININT_OFF EIMSK &= ~_BV(CC1100_INT)
#endif
#endif // XMEGA
