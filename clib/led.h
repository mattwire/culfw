#ifndef _LED_H
#define _LED_H   1

#define HI8(x)  ((uint8_t)((x) >> 8))
#define LO8(x)  ((uint8_t)(x))

#ifdef HAS_RGBLED
uint8_t rgb_led[3];
uint8_t led_on;
uint8_t led_fade;
#endif

#include "board.h"

#ifdef XMEGA

// XMEGA code
#ifdef HAS_RGBLED
#include "xled.h"
#define led_init()  xled_pos=0; xled_pattern=0x8000; led_fade = 0;
#define LED_TOGGLE() led_on ^= 0xff
#define LED_OFF()  led_on = 0
#define LED_ON()   led_on = 1; led_fade = 0
#else
#define led_init() LED_PORT.DIRSET = LED_PIN
#define LED_TOGGLE() LED_PORT.OUTTGL = LED_PIN
#define LED_OFF()  LED_PORT.OUTCLR = LED_PIN
#define LED_ON()   LED_PORT.OUTSET = LED_PIN
#endif

#else
// AVR8 code

#define SET_BIT(PORT, BITNUM) ((PORT) |= (1<<(BITNUM)))
#define CLEAR_BIT(PORT, BITNUM) ((PORT) &= ~(1<<(BITNUM)))
#define TOGGLE_BIT(PORT, BITNUM) ((PORT) ^= (1<<(BITNUM)))

#ifdef XLED
#include "xled.h"
#define led_init()   LED_DDR  |= _BV(LED_PIN); xled_pos=0; xled_pattern=0xff00
#else
#define led_init()   LED_DDR  |= _BV(LED_PIN)
#endif

#define LED_TOGGLE() LED_PORT ^= _BV(LED_PIN)

#ifdef LED_INV
#define LED_OFF()    LED_PORT |= _BV(LED_PIN)
#define LED_ON( )    LED_PORT &= ~_BV(LED_PIN)
#else
#define LED_ON()     LED_PORT |= _BV(LED_PIN)
#define LED_OFF( )   LED_PORT &= ~_BV(LED_PIN)
#endif

#endif
#endif
