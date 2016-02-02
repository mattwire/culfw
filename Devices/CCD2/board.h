#ifndef _BOARD_H
#define _BOARD_H

// Feature definitions
#define BOARD_ID_STR            "CSM868"
#define BOARD_ID_USTR           L"CSM868"

#define MULTI_FREQ_DEVICE	// available in multiple versions: 433MHz,868MHz
#define BOARD_ID_STR433         "CSM433"
#define BOARD_ID_USTR433        L"CSM433"

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
#define HAS_MBUS
#define HAS_RFNATIVE
#define HAS_ZWAVE
#define LACROSSE_HMS_EMU

#define TTY_BUFSIZE          128
#define _HAS_PIGATOR

#include <avr/io.h>
#include <avr/power.h>

#define XMEGA

#define HAS_UART              USARTE0  
#define UART_BAUD_RATE        38400
#define UART_PORT             PORTE
#define UART_DRE_vect         USARTE0_DRE_vect
#define UART_RXC_vect         USARTE0_RXC_vect
#define RPI_TTY_FIX

#define CC1100_CS_PORT        PORTD
#define CC1100_CS_PIN	      PIN4_bm

#define CC1100_OUT_PORT       PORTE
#define CC1100_OUT_PIN        PIN0_bm

#define CC1100_IN_PORT        PORTE
#define CC1100_IN_PIN         PIN1_bm
#define CC1100_IN_PINCTRL     PIN1CTRL
#define CC1100_IN_INTMASK     INT1MASK
#define CC1100_IN_INT         PORTE_INT1_vect

#define CC1100_SPI_PORT       PORTD
#define CC1100_SPI	      SPID

#define LED_PORT              PORTD
#define LED_PIN               PIN2_bm

#define HAS_PIGATOR
#define PIG_STACKED
#define PIG_UART              USARTC0
#define PIG_UART_PORT         PORTC
#define PIG_DRE_vect          USARTC0_DRE_vect
#define PIG_RXC_vect          USARTC0_RXC_vect
#define PIG_TWI               TWIC
#define PIG_SPI               SPIC
#define PIG_RESET_PORT        PORTB
#define PIG_RESET_PIN         PIN2_bm
#define PIG_BSEL_PORT         PORTB
#define PIG_BSEL_PIN          PIN1_bm
#define PIG_RTS_PORT          PORTB
#define PIG_RTS_PIN           PIN3_bm

#define CUL_HW_REVISION       "CCD_V2"

#endif // __BOARD_H__
