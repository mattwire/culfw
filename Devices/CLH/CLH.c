/* Copyright Rudolf Koenig, 2008.
   Released under the GPL Licence, Version 2
   Inpired by the MyUSB USBtoSerial demo, Copyright (C) Dean Camera, 2008.
*/

#include <avr/io.h>

#include "board.h"
#include "culfw.h"
#include "led.h"
#include "fncollection.h"

void setup() {
  BLED_PORT.OUTSET = BLED_PIN;
  BLED_PORT.DIRSET = BLED_PIN;
  RLED_PORT.OUTSET = RLED_PIN;
  RLED_PORT.DIRSET = RLED_PIN;
  GLED_PORT.OUTSET = GLED_PIN;
  GLED_PORT.DIRSET = GLED_PIN;

  rgb_led[0] = 0;
  rgb_led[1] = 255;
  rgb_led[2] = 0;
}

void loop() {
}
