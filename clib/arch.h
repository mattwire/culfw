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

#define BITCNT TCD0.CNT
#define BITMAX TCD0.PER
#define BITINT TCD0_OVF_vect
#define BITINT_OFF TCD0.INTCTRLA = 0
#define BITINT_ON  TCD0.INTCTRLA = 1
#define BITINT_CLR TCD0.INTFLAGS |= 1

#define CC1100_OUT_SET CC1100_OUT_PORT.OUTSET = CC1100_OUT_PIN
#define CC1100_OUT_CLR CC1100_OUT_PORT.OUTCLR = CC1100_OUT_PIN
#define PININT CC1100_IN_INT
#define PININT_ON CC1100_IN_PORT.CC1100_IN_INTMASK = CC1100_IN_PIN
#define PININT_OFF CC1100_IN_PORT.CC1100_IN_INTMASK = 0
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
