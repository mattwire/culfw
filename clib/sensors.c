#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "board.h"
#ifdef HAS_SENSORS
#include "clock.h"
#include "sensors.h"
#include "display.h"
#include "rf_receive.h"
#include "registry.h"

#include "LUFA/Drivers/Peripheral/TWI.h"

static uint32_t sens_ticks = 0;
static uint8_t tsl_sens = 0;
uint8_t tsl_version  = 0;
uint16_t sens_interval = 0;

static void config_TSL(void) {
  uint8_t RegAddress  = TSL_REG_CONFIG;
  uint8_t WritePacket = (tsl_sens & 3);

  if (tsl_version)
    TWI_WritePacket(&SENSORS_TWI, TSL_ADDR, 10, &RegAddress, 1, &WritePacket, 1); // set multiplier
}

void sensors_init(void) {

  TWI_Init(&SENSORS_TWI, TWI_BAUD_FROM_FREQ(100000));
  SENSORS_TWI.CTRL |= TWI_SDAHOLD_50NS_gc; // Onewire PIM w/ level changers requires this

#ifdef SENSORS_I2C_PULL
  // Enable internal pull-up on PIN0, PIN1..
  PORTCFG.MPCMASK = 0x03; // Configure several PINxCTRL registers at the same time
  SENSORS_I2C_PULL.PIN0CTRL = (SENSORS_I2C_PULL.PIN0CTRL & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc; //Enable pull-up to get a defined level on the switches
#endif  
  
  uint8_t RegAddress  = TSL_REG_CONTROL;
  uint8_t WritePacket = 3;
  if (TWI_WritePacket(&SENSORS_TWI, TSL_ADDR, 10, &RegAddress, 1, &WritePacket, 1) != TWI_ERROR_NoError) {// Pwr On TSL
    // DS_P(PSTR("cfg error\r\n"));
  }

  RegAddress = TSL_REG_ID;
  if (TWI_ReadPacket(&SENSORS_TWI, TSL_ADDR, 10, &RegAddress, 1, &tsl_version, 1) == TWI_ERROR_NoError) {
    tsl_version = (tsl_version & 0xF0) >> 4;
    //    DC('V'); DH2(tsl_version); DNL();
  } else {
    tsl_version = 0;
    //DS_P(PSTR("id error\r\n"));
  }  
  
  tsl_sens = 0;
  config_TSL();

  sens_interval = 0;
  
  if (registry_get( REG_BRIGHTNESS_INTERVAL, &sens_interval ) != REG_STATUS_OK)
    return;
  
  sens_ticks = -1;
  if (!sens_interval)
    return;

  sens_ticks = ticks + (sens_interval * 125);
}

static uint32_t read_TSL(void) {
  uint8_t  RegAddress  = TSL_REG_DATALOW;
  uint16_t ReadPacket = 0;
  
  TWI_ReadPacket(&SENSORS_TWI, TSL_ADDR, 10, &RegAddress, 1, &ReadPacket, 2);

  return ReadPacket;
}

  
void sensors_task(void) {
  uint32_t lux;

  if (ticks < sens_ticks)
    return;

  if (tsl_version) {

    lux = read_TSL();
    sens_ticks = ticks + (1 * 125); // come back in a second ...
    
    // automatically adjust range
    if (((lux & 0xc000) == 0xc000) && (tsl_sens<2)) {
      tsl_sens++;
      config_TSL();
      return;
    }
    
    if (!(lux & 0xc000) && (tsl_sens>0)) {
      tsl_sens--;
      config_TSL();
      return;
    }

    lux = (lux<<tsl_sens);

    // find good factor
    uint8_t f = 0;
    while (lux > 4095) {
      f++;
      lux = lux/10;
    }
    
    // K75123fCS
    uint8_t hb[4] = { 0x75, 0x12, 0x34, 0xff };

    hb[1]  = (lux & 0xf)<<4;
    hb[1] |= (lux & 0xf0)>>4;
  
    hb[2]  = f;
    hb[2] |= (lux & 0xf00)>>4;
    
    // calc checksum
    hb[3] = cksum3( hb, 3 );
    DC('K');
    for (uint8_t i=0;i<4;i++)
      DH2( hb[i] );
    DNL();
    
  }

  sens_ticks = -1;
  if (sens_interval)
    sens_ticks = ticks + (sens_interval * 125);
    
}

void sensors_func(char *in) {
  if (in[1] == 'i') {
    if (in[2]) {     // set interval
      fromdec(in+2, (uint8_t *) &sens_interval);
      registry_set( REG_BRIGHTNESS_INTERVAL, &sens_interval, 2 );
    }
    DH2(sens_interval); DNL();
  }

  sens_ticks = ticks-1;
}

#endif
