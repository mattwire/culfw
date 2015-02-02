#include "board.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#include "arch.h"
#include "led.h"
#include "fncollection.h"

#ifdef LUFA
#include <LUFA/Platform/Platform.h>
#endif


void cpu_set_clock(void) {

#ifdef LUFA
  // use LUFA clock management if available
  /* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
  XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
  XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);
  
  /* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
  XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
  XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);
  
#else
  CCP = CCP_IOREG_gc;              // disable register security for oscillator update
  OSC.CTRL = OSC_RC32MEN_bm;       // enable 32MHz oscillator
  while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator to be ready
  CCP = CCP_IOREG_gc;              // disable register security for clock update
  CLK.CTRL = CLK_SCLKSEL_RC32M_gc; // switch to 32MHz clock
#endif

  PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

  // setup watchdog
  uint8_t temp = WDT_ENABLE_bm | WDT_CEN_bm | WDT_PER_1KCLK_gc;
  CCP = CCP_IOREG_gc;
  WDT.CTRL = temp;
  
  /* Wait for WD to synchronize with new settings. */
  // while(WDT_IsSyncBusy());
    
}

void start_bootloader(void) {
  
  /* Jump to 0x401FC = BOOT_SECTION_START + 0x1FC which is
   * the stated location of the bootloader entry (AVR1916).
   * Note the address used is the word address = byte addr/2.
   * My ASM fu isn't that strong, there are probably nicer
   * ways to do this with, yennow, defined symbols .. */
  
  asm ("ldi r30, 0xFE\n"  /* Low byte to ZL */
       "ldi r31, 0x00\n" /* mid byte to ZH */
       "ldi r24, 0x02\n" /* high byte to EIND which lives */
       "out 0x3c, r24\n" /* at addr 0x3c in I/O space */
       "eijmp":  :: "r24", "r30", "r31");
  
}

void check_bootloader_req(void) {
  // if we had been restarted by watchdog check the REQ BootLoader byte in the
  // EEPROM ...
  if (bit_is_set(RST.STATUS,3) && erb(EE_REQBL)) {
    ewb( EE_REQBL, 0 ); // clear flag
    //    start_bootloader();
  }
}

uint8_t is433MHz(void) {
  return 0;
}

void spi_init(void) {
  CC1100_CS_PORT.DIRSET = CC1100_CS_PIN; 
  CC1100_CS_PORT.OUTSET = CC1100_CS_PIN; 

  CC1100_SPI_PORT.DIRSET = PIN5_bm | PIN7_bm;  // MOSI, SCK outputs
  CC1100_SPI_PORT.DIRCLR = PIN6_bm;  // MISO input
  CC1100_SPI.CTRL = (SPI_MODE_0_gc | SPI_PRESCALER_DIV16_gc | SPI_ENABLE_bm | SPI_MASTER_bm);
  //  CC1100_SPI.INTCTRL |= SPI_INTLVL_LO_gc;  //interrupts
}

uint8_t spi_send(uint8_t data) {
  //  CC1100_SPI.STATUS;
  CC1100_SPI.DATA = data;
  while((CC1100_SPI.STATUS & SPI_IF_bm) == 0x00); //wait completion
  return CC1100_SPI.DATA;
}

uint8_t gdo0_is_set(void) {
  return ( CC1100_OUT_PORT.IN & CC1100_OUT_PIN ) ? 1 : 0;
}

uint8_t gdo2_is_set(void) {
  return ( CC1100_IN_PORT.IN & CC1100_IN_PIN ) ? 1 : 0;
}

/*********
 * Delay *
 *********/

static void my_delayInit(void) {
  TCE0.CTRLA = 0x01;						//set clock/1
  TCE0.CTRLB = 0x01; //0x31;						//enable COMA and COMB, set to FRQ
  TCE0.INTCTRLB = 0x00;					//turn off interrupts for COMA and COMB
  SREG |= CPU_I_bm;						//enable all interrupts
  PMIC.CTRL |= 0x01;						//enable all low priority interrupts
}


void my_delay_us(uint16_t cnt) {
  /*
  delaycnt = 0;							//set counter
  TCE0.CCA = 32;							//set COMA to be 1us delay
  TCE0.CNT = 0;							//reset counter
  TCE0.INTCTRLB = 0x01;					//enable low priority interrupt for delay
  while (cnt != delaycnt);				//delay
  TCE0.INTCTRLB = 0x00;					//disable interrupts
  */

  TCE0.CTRLA = 0;
  TCE0.CTRLB = 0;
  TCE0.PER  = cnt;
  TCE0.CNT   = 0;
  TCE0.INTFLAGS |= 1;
  TCD0.INTCTRLA = 0x01;					        //enable OVFINT
  TCE0.CTRLA = TC_CLKSEL_EVCH0_gc;
  while (1) 
    if (TCE0.INTFLAGS & 1)
      break;

  TCE0.CTRLA = 0;
  
}	

void my_delay_ms(uint16_t cnt) {
  /*
  delaycnt = 0;							//set count value
  TCE0.CCA = 32000;						//set COMA to be 1ms delay
  TCE0.CNT = 0;							//reset counter
  TCE0.INTCTRLB = 0x01;					//enable low priority interrupt for delay
  while (cnt != delaycnt);				//delay
  TCE0.INTCTRLB = 0x00;					//disable interrupts
  */
  while(cnt--) {
    my_delay_us( 1000 );
  }

}	

/*
SIGNAL(TCE0_CCA_vect) {
  delaycnt++;
}

SIGNAL(TCE0_CCB_vect) {
  delaycnt++;
}
*/

// SETUP

void setup_timer(void) {

  // Delay uses E0 timer
  //  my_delayInit();

  // 125Hz clock uses C0 timer
  TCC0.CNT   = 0;
  TCC0.CTRLA = 0x07;						//set F_CPU/1024/250 = 125Hz
  TCC0.CCA   = 250;
  
  TCC0.CTRLB = 0x01;					        //set to FRQ
  TCC0.INTCTRLB = 0x01;					        //enable INT

  // 1us clock uses C1 timer - and drives EVENT CH 0
  TCC1.CNT   = 0;
  TCC1.CTRLA = 0x01;						//set F_CPU/1/32 = 1MHz = 1us
  TCC1.PER   = 32;

  EVSYS.CH0MUX = EVSYS_CHMUX_TCC1_OVF_gc;

  // 1us per tick - for BIT-TIMING
  TCD0.CNT   = 0;
  TCD0.CTRLA = TC_CLKSEL_EVCH0_gc;
  TCD0.PER   = 50000;
  BITINT_OFF;
}

void setup_io(void) {
}

