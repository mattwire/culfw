#ifndef _PIGATOR_H
#define _PIGATOR_H

void pigator_init(void);
void pigator_task(void);
void pigator_func(char *in);

#define USART_Baudrate_Set(_usart, _bselValue, _bScaleFactor)                  \
	(_usart)->BAUDCTRLA =(uint8_t)_bselValue;                              \
	(_usart)->BAUDCTRLB =(_bScaleFactor << USART_BSCALE0_bp)|(_bselValue >> 8)

#define USART_RxdInterruptLevel_Set(_usart, _rxdIntLevel)                      \
	((_usart)->CTRLA = ((_usart)->CTRLA & ~USART_RXCINTLVL_gm) | _rxdIntLevel)

#define USART_DreInterruptLevel_Set(_usart, _dreIntLevel)                      \
	(_usart)->CTRLA = ((_usart)->CTRLA & ~USART_DREINTLVL_gm) | _dreIntLevel


typedef struct {
  char     Magic[10];
  uint16_t Baud;
  void(* cb_mod_init)(void);
  void(* cb_mod_task)(void);
  void(* cb_mod_reset)(uint8_t on);
  void(* cb_mod_bootload)(void);
} Pigator_Module_t;

#endif

