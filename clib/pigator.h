#ifndef _PIGATOR_H
#define _PIGATOR_H

#define USART_BUF_SIZE 256

void pigator_init(void);
void pigator_task(void);
void pigator_func(char *in);
void pigator_stack_func(char *in);

void toPIMBuffer(uint8_t in);

typedef struct {
  char     Magic[10];
  uint32_t Baud;
  void(* cb_mod_init)(void);
  void(* cb_mod_task)(void);
  void(* cb_mod_reset)(uint8_t on);
  void(* cb_mod_bootload)(void);
} Pigator_Module_t;

#endif

