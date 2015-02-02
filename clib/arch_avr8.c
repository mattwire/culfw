#include "board.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "util/delay_basic.h"

#include "arch.h"
#include "fncollection.h"

void cpu_set_clock(void) {
  wdt_enable(WDTO_2S);
  clock_prescale_set(clock_div_1);
}

void
start_bootloader(void)
{
  cli();

  /* move interrupt vectors to bootloader section and jump to bootloader */
  MCUCR = _BV(IVCE);
  MCUCR = _BV(IVSEL);

#if defined(CUL_V3) || defined(CUL_V4)
#  define jump_to_bootloader ((void(*)(void))0x3800)
#endif
#if defined(CUL_V2)
#  define jump_to_bootloader ((void(*)(void))0x1800)
#endif
  jump_to_bootloader();
}

void
check_bootloader_req(void)
{
  // if we had been restarted by watchdog check the REQ BootLoader byte in the
  // EEPROM ...
  if(bit_is_set(MCUSR,WDRF) && erb(EE_REQBL)) {
    ewb( EE_REQBL, 0 ); // clear flag
    start_bootloader();
  }
}


void
setup_io(void)
{
  MARK433_PORT |= _BV( MARK433_BIT ); // Pull 433MHz marker
  MARK915_PORT |= _BV( MARK915_BIT ); // Pull 915MHz marker
}

uint8_t is433MHz(void) {
  return bit_is_set(MARK433_PIN, MARK433_BIT) ? 0 : 1;
}

void
setup_timer(void)
{
  // Setup the timers. Are needed for watchdog-reset
  OCR0A  = 249;                            // Timer0: 0.008s = 8MHz/256/250 == 125Hz
  TCCR0B = _BV(CS02);
  TCCR0A = _BV(WGM01);
  TIMSK0 = _BV(OCIE0A);

  TCCR1A = 0;
  TCCR1B = _BV(CS11) | _BV(WGM12);         // Timer1: 1us = 8MHz/8 == 1MHz
}

void
spi_init(void)
{
#ifdef PRR0
  PRR0 &= ~_BV(PRSPI);
#endif
  SPI_PORT |= _BV(SPI_SCLK);
  SPI_DDR  |= (_BV(SPI_MOSI) | _BV(SPI_SCLK) | _BV(SPI_SS));
  SPI_DDR  &= ~_BV(SPI_MISO);
  
#ifdef HAS_DOGM
  SPCR = _BV(MSTR) | _BV(SPE) | _BV( SPR0 );
#else
  SPCR  = _BV(MSTR) | _BV(SPE);
  SPSR |= _BV(SPI2X);
#endif

}

uint8_t
spi_send(uint8_t data)
{
  SPDR = data;
  while (!(SPSR & _BV(SPIF)));
  return SPDR;
}

uint8_t gdo0_is_set(void) {
  return bit_is_set( CC1100_OUT_IN, CC1100_OUT_PIN );
}

uint8_t gdo2_is_set(void) {
  return bit_is_set( CC1100_IN_IN, CC1100_IN_PIN );
}

/*********
 * Delay *
 *********/

void my_delay_us( uint16_t d ) {
#if 1
  d<<=1;                // 4 cycles/loop, we are running 8MHz
  _delay_loop_2(d);    
#else
  TIMSK1 = 0;           // No interrupt if counter is reached
  TCNT1 = 0;            // The timer must be set up to run with 1MHz
  while (TCNT1<d) {};
#endif
}

void my_delay_ms( uint16_t d ) {
  while(d--) {
    my_delay_us( 1000 );
  }
}
