#ifndef _BOARD_H
#define _BOARD_H

// Feature definitions
#define BOARD_ID_STR            "CUL868"
#define BOARD_ID_USTR           L"CUL868"

#define HAS_FHT_80b                     // PROGMEM: 1374b, RAM: 90b
#define HAS_RF_ROUTER                   // PROGMEM: 1248b  RAM: 44b
#define HAS_CC1101_RX_PLL_LOCK_CHECK_TASK_WAIT	// PROGMEM: 118b
#define HAS_CC1101_PLL_LOCK_CHECK_MSG		// PROGMEM:  22b
#define HAS_CC1101_PLL_LOCK_CHECK_MSG_SW	// PROGMEM:  22b

#define HAS_FHT_8v                    // PROGMEM:  586b  RAM: 23b
#define HAS_FHT_TF
#define FHTBUF_SIZE          174      //                 RAM: 174b
#define RCV_BUCKETS            4      //                 RAM: 25b * bucket
#define RFR_DEBUG                     // PROGMEM:  354b  RAM: 14b
#define FULL_CC1100_PA                // PROGMEM:  108b
#define HAS_RAWSEND                   //
#define HAS_FASTRF                    // PROGMEM:  468b  RAM:  1b
#define HAS_ASKSIN
#define HAS_ASKSIN_FUP
#define HAS_MORITZ
#define HAS_ESA
#define HAS_TX3
#define HAS_INTERTECHNO
#define HAS_UNIROLL
#define HAS_HOERMANN
#define HAS_MEMFN
#define HAS_SOMFY_RTS
#define HAS_KOPP_FC
#define HAS_NATIVE
#define HAS_MBUS
#define HAS_ZWAVE
#define LACROSSE_HMS_EMU

#define TTY_BUFSIZE          128

#include <avr/io.h>
#include <avr/power.h>

#define XMEGA

#define HAS_USB               1
#define USB_BUFSIZE           64      // Must be a supported USB endpoint size
#define USB_MAX_POWER	      100

#define CC1100_CS_PORT        PORTC
#define CC1100_CS_PIN	      PIN4_bm

#define CC1100_OUT_PORT       PORTA
#define CC1100_OUT_PIN        PIN4_bm

#define CC1100_IN_PORT        PORTA
#define CC1100_IN_PIN         PIN5_bm
#define CC1100_IN_PINCTRL     PIN5CTRL
#define CC1100_IN_INTMASK     INT1MASK
#define CC1100_IN_INT         PORTA_INT1_vect

#define CC1100_SPI_PORT       PORTC
#define CC1100_SPI	      SPIC

#define LED_PORT              PORTE
#define LED_PIN               PIN2_bm

#define HAS_PIGATOR
#define PIG_UART              USARTD0
#define PIG_UART_PORT         PORTD
#define PIG_DRE_vect          USARTD0_DRE_vect
#define PIG_RXC_vect          USARTD0_RXC_vect
#define PIG_TXC_vect          USARTD0_TXC_vect
#define PIG_TWI               TWIE
#define PIG_I2C_PULL          PORTE
#define PIG_SPI               CC1100_SPI
#define PIG_SPI_SS_PORT       PORTC
#define PIG_SPI_SS_PIN        PIN0_bm
#define PIG_RESET_PORT        PORTA
#define PIG_RESET_PIN         PIN0_bm
#define PIG_BSEL_PORT         PORTA
#define PIG_BSEL_PIN          PIN3_bm
#define PIG_RTS_PORT          PORTD
#define PIG_RTS_PIN           PIN5_bm

#define HAS_W5500
#define W5500_RESET_PORT      PORTD
#define W5500_RESET_PIN       PIN1_bm
#define W5500_CS_PORT         PORTD
#define W5500_CS_PIN          PIN0_bm

#define HAS_MQTT

#define CUL_HW_REVISION       "CUNX_V1"

#endif // __BOARD_H__
