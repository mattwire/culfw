#include <avr/io.h>
#include <avr/pgmspace.h>

#include "culfw.h"

#include "cc1100.h"
#include "clock.h"
#include "arch.h"
#include "display.h"
#include "fncollection.h"
#include "led.h"		// ledfunc
#include "ringbuffer.h"
#include "rf_receive.h"
#include "rf_send.h"		// fs20send
#include "ttydata.h"
#include "fht.h"		// fhtsend
#include "fastrf.h"		// fastrf_func
#include "rf_router.h"		// rf_router_func

#ifdef HAS_MEMFN
#include "memory.h"		// getfreemem
#endif
#ifdef HAS_USB
#include "usb.h"
#endif
#ifdef HAS_ASKSIN
#include "rf_asksin.h"
#endif
#ifdef HAS_MORITZ
#include "rf_moritz.h"
#endif
#ifdef HAS_RWE
#include "rf_rwe.h"
#endif
#ifdef HAS_INTERTECHNO
#include "intertechno.h"
#endif
#ifdef HAS_SOMFY_RTS
#include "somfy_rts.h"
#endif
#ifdef HAS_MBUS
#include "rf_mbus.h"
#endif
#ifdef HAS_KOPP_FC
#include "kopp-fc.h"
#endif
#ifdef HAS_PIGATOR
#include "pigator.h"
#endif
#ifdef HAS_W5500
#include "ethernet.h"
#endif

const PROGMEM t_fntab fntab[] = {

  { 'B', prepare_boot },
#ifdef HAS_MBUS
  { 'b', rf_mbus_func },
#endif
  { 'C', ccreg },
  { 'F', fs20send },
#ifdef HAS_INTERTECHNO
  { 'i', it_func },
#endif
#ifdef HAS_KOPP_FC
{ 'k', kopp_fc_func },
#endif
#ifdef HAS_ASKSIN
  { 'A', asksin_func },
#endif
#ifdef HAS_PIGATOR
  { 'p', pigator_func },
#endif
#ifdef HAS_MORITZ
  { 'Z', moritz_func },
#endif
#ifdef HAS_RWE
  { 'E', rwe_func },
#endif
#ifdef HAS_RAWSEND
  { 'G', rawsend },
  { 'M', em_send },
  { 'K', ks_send },
#endif
#ifdef HAS_UNIROLL
  { 'U', ur_send },
#endif
#ifdef HAS_SOMFY_RTS
  { 'Y', somfy_rts_func },
#endif
  { 'R', read_eeprom },
  { 'T', fhtsend },
  { 'V', version },
  { 'W', write_eeprom },
  { 'X', set_txreport },

  { 'e', eeprom_factory_reset },
#ifdef HAS_FASTRF
  { 'f', fastrf_func },
#endif
#ifdef HAS_MEMFN
  { 'm', getfreemem },
#endif
  { 'l', ledfunc },
  { 't', gettime },
#ifdef HAS_RF_ROUTER
  { 'u', rf_router_func },
#endif
  { 'x', ccsetpa },
#ifdef HAS_W5500
  { 'E', ethernet_func },
#endif

  { 0, 0 },
};

int display_putchar (char data, FILE * stream) {
  display_char( data );
}

int main(void) {

  check_bootloader_req();

  cpu_set_clock();
  setup_io();

  setup_timer();

  fdevopen(&display_putchar,NULL); // make printf work

  led_init();
  spi_init();
  eeprom_init();
#ifdef HAS_USB
  usb_init();
#endif
#ifdef HAS_W5500
  ethernet_init();
#endif
#ifdef HAS_PIGATOR
  pigator_init();
#endif
  fht_init();
  tx_init();

  input_handle_func = analyze_ttydata;
#ifdef HAS_RF_ROUTER
  rf_router_init();
  display_channel = (DISPLAY_USB|DISPLAY_RFROUTER);
#else
  display_channel = DISPLAY_USB;
#endif

#ifdef HAS_W5500
  display_channel |= DISPLAY_TCP;
#endif
  
  LED_OFF();

  setup();
  
  for(;;) {
#ifdef HAS_USB
    usb_task();
#endif
#ifdef HAS_W5500
    ethernet_task();
#endif
    RfAnalyze_Task();
    Minute_Task();
#ifdef HAS_FASTRF
    FastRF_Task();
#endif
#ifdef HAS_RF_ROUTER
    rf_router_task();
#endif
#ifdef HAS_ASKSIN
    rf_asksin_task();
#endif
#ifdef HAS_MORITZ
    rf_moritz_task();
#endif
#ifdef HAS_RWE
    rf_rwe_task();
#endif
#ifdef HAS_MBUS
    rf_mbus_task();
#endif
#ifdef HAS_PIGATOR
  pigator_task();
#endif

    loop();

  }

}
