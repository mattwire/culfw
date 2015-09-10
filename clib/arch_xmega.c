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
#include <LUFA/Drivers/Peripheral/Serial.h>
#endif

void setUpOsc() {
  
  
}

void cpu_set_clock(void) {


#ifdef LUFA
  /* start internal reference */
  XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32KHZ);

#ifdef HAS_USB
  /* Start the PLL for 48MHz USB clocking */
  XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC2MHZ);
  XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC2MHZ, DFLL_REF_INT_RC32KHZ, 2000000);
  XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_USB);
#endif
  
  /* Start the 32MHz internal RC oscillator and set as system clock */
  XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
  XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_RC32KHZ, F_CPU);
  XMEGACLK_SetCPUClockSource(CLOCK_SRC_INT_RC32MHZ);

#else

  // Configure clock to 32MHz
  OSC.CTRL |= OSC_RC32MEN_bm | OSC_RC32KEN_bm;  /* Enable the internal 32MHz & 32KHz oscillators */
  while(!(OSC.STATUS & OSC_RC32KRDY_bm));       /* Wait for 32Khz oscillator to stabilize */
  while(!(OSC.STATUS & OSC_RC32MRDY_bm));       /* Wait for 32MHz oscillator to stabilize */
  DFLLRC32M.CTRL = DFLL_ENABLE_bm ;             /* Enable DFLL - defaults to calibrate against internal 32Khz clock */
  CCP = CCP_IOREG_gc;                           /* Disable register security for clock update */
  CLK.CTRL = CLK_SCLKSEL_RC32M_gc;              /* Switch to 32MHz clock */
  OSC.CTRL &= ~OSC_RC2MEN_bm;                   /* Disable 2Mhz oscillator */

#endif

  PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

  // setup watchdog
  uint8_t temp = WDT_ENABLE_bm | WDT_CEN_bm | WDT_PER_2KCLK_gc;
  CCP = CCP_IOREG_gc;
  WDT.CTRL = temp;
  
  /* Wait for WD to synchronize with new settings. */
  while(WDT.STATUS & WDT_SYNCBUSY_bm);
    
}

void(* call_bootloader)(void) = (void (*)(void))(BOOT_SECTION_START/2+0x1FC/2);

void start_bootloader(void) {
  EIND = BOOT_SECTION_START>>17;
  call_bootloader();
}

void check_bootloader_req(void) {
  // if we had been restarted by watchdog check the REQ BootLoader byte in the
  // EEPROM ...
  if (bit_is_set(RST.STATUS,3) && erb(EE_REQBL)) {
    RST.STATUS |= _BV(3);
    ewb( EE_REQBL, 0 ); // clear flag
    start_bootloader();  
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
  /*
  TCE0.CTRLA = 0x01;						//set clock/1
  TCE0.CTRLB = 0x01; //0x31;						//enable COMA and COMB, set to FRQ
  TCE0.INTCTRLB = 0x00;					//turn off interrupts for COMA and COMB
  SREG |= CPU_I_bm;						//enable all interrupts
  PMIC.CTRL |= 0x01;						//enable all low priority interrupts
  */
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
  TCE0.CTRLA = TC_CLKSEL_EVCH0_gc;
  while (1) {
    if (TCE0.INTFLAGS & 1)
      break;
    wdt_reset();
  }

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

  if (cnt<65) {
    my_delay_us( cnt * 1000 );
    return;
  }
  
  while(cnt--) {
    my_delay_us( 1000 );
  }

}	

// SETUP

void setup_timer(void) {

  // Delay uses E0 timer
  //  my_delayInit();

  // 125Hz system clock uses C0 timer
  TCC0.CNT   = 0;
  TCC0.CTRLA = TC_CLKSEL_EVCH0_gc; // 1us = 1MHz
#if defined (HAS_IRX)  || defined (HAS_IRTX)
  TCC0.CCA   = 50-1;   // 20000 Hz - to call irrx or irtx ISR
#else
  TCC0.CCA   = 8000-1; // 125 Hz
#endif
  
  TCC0.CTRLB = 0x01;					        //set to FRQ
  TCC0.INTCTRLB = 0x01;					        //enable INT

  // 1us clock uses C1 timer - and drives EVENT CH 0
  TCC1.CNT   = 0;
  TCC1.CTRLA = 0x01;						//set F_CPU/1/32 = 1MHz = 1us
  TCC1.PER   = 32-1;

  EVSYS.CH0MUX = EVSYS_CHMUX_TCC1_OVF_gc;

  // D0 is used for IR-TX FRQ generation on TCD0.CCA  i.e. 36kHz
  
  // D1 1us per tick - for SlowRF BIT-TIMING
  BITTC.CNT   = 0;
  BITTC.CTRLA = TC_CLKSEL_EVCH0_gc;
  BITTC.PER   = 50000;
  BITINT_OFF;
}

void setup_io(void) {
}

void mySerial_Init(USART_t* const USART, const uint32_t BaudRate) {
  int8_t bscale = 4;
  uint16_t bsel  = 12;

  Serial_Init(USART, BaudRate, false); // this is not setting Baudrate precise enough!

  switch (BaudRate) {
  case 300:
    bsel   = 207;
    bscale = 5;
    break;
  case 600:
    bsel   = 25;
    bscale = 7;
    break;
  case 1200:
    bsel   = 12;
    bscale = 7;
    break;
  case 2400:
    bsel   = 12;
    bscale = 6;
    break;
  case 4800:
    bsel   = 12;
    bscale = 5;
    break;
  case 9600:
    bsel   = 12;
    bscale = 4;
    break;
  case 14400:
    bsel   = 138;
    bscale = 0;
    break;
  case 19200:
    bsel   = 12;
    bscale = 3;
    break;
  case 28800:
    bsel   = 137;
    bscale = -1;
    break;
  case 38400:
    bsel   = 12;
    bscale = 2;
    break;
  case 38461:
    bsel   = 3290;
    bscale = -6;
    break;
  case 57600:
    bsel   = 135;
    bscale = -2;
    break;
  case 76800:
    bsel   = 12;
    bscale = 1;
    break;
  case 115200:
    bsel   = 131;
    bscale = -3;
    break;
  case 230400:
    bsel   = 123;
    bscale = -4;
    break;
  case 460800:
    bsel   = 107;
    bscale = -5;
    break;
  default:
    return;
  }
  
  USART_Baudrate_Set(USART, bsel, bscale);
}
