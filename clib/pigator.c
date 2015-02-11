#include "board.h"
#ifdef HAS_PIGATOR

#include "pigator.h"
#include "LUFA/Drivers/Peripheral/TWI.h"
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>

#include "Descriptors.h"
#include "usb.h"
#include "arch.h"
#include "string.h"
#include "display.h"
#include "led.h"
#include <avr/pgmspace.h>

#define USART USARTD0
#define USART_BUF_SIZE 128

static RingBuffer_t USBtoUSART_Buffer;
static uint8_t      USBtoUSART_Buffer_Data[USART_BUF_SIZE];
static RingBuffer_t USARTtoUSB_Buffer;
static uint8_t      USARTtoUSB_Buffer_Data[USART_BUF_SIZE];

Pigator_Module_t *Pigator_Module = NULL;

#define EEP_ADDR       0xA0
static char EEP_MAGIC[10];

void(* pig_mod_task)(void) = NULL;
void(* pig_mod_init)(void) = NULL;
void(* pig_mod_reset)(uint8_t on) = NULL;
void(* pig_mod_bootload)(void) = NULL;

void pig_reset_low(uint8_t on) {

  PORTA.DIRSET = PIN0_bm;

  if (on) {
    PORTA.OUTCLR = PIN0_bm;
  } else
    PORTA.OUTSET = PIN0_bm;
  
}

void pig_reset_high(uint8_t on) {

  PORTA.DIRSET = PIN0_bm;

  if (on) {
    PORTA.OUTSET = PIN0_bm;
  } else
    PORTA.OUTCLR = PIN0_bm;
  
}

void reset_module(void) {
  if (pig_mod_reset) {
    pig_mod_reset( 1 );
    my_delay_ms(10);
    pig_mod_reset( 0 );
    my_delay_ms(10);
  }
}

void pig_csm_bootload(void) {
  PORTA.DIRSET = PIN3_bm;
  pig_mod_reset( 1 );
  PORTA.OUTCLR = PIN3_bm;
  my_delay_ms(10);
  pig_mod_reset( 0 );
  my_delay_ms(500);
  PORTA.OUTSET = PIN3_bm;
  PORTA.DIRCLR = PIN3_bm;
}

void pig_serialfwd_init(void) {

  if (!Pigator_Module)
    return;

  reset_module();
  
  RingBuffer_InitBuffer(&USBtoUSART_Buffer, USBtoUSART_Buffer_Data, sizeof(USBtoUSART_Buffer_Data));
  RingBuffer_InitBuffer(&USARTtoUSB_Buffer, USARTtoUSB_Buffer_Data, sizeof(USARTtoUSB_Buffer_Data));

  // USART RX/TX 1
  /* PIN3 (TXD0) as output. */
  PORTD.DIRSET = PIN3_bm;
  /* PC2 (RXD0) as input. */
  PORTD.DIRCLR = PIN2_bm;
  
  Serial_Init(&USART, Pigator_Module->Baud, false);
  USART_RxdInterruptLevel_Set(&USART, USART_RXCINTLVL_LO_gc);
}

void pig_serialfwd_task(void) {

  // USB => PIGATOR
  while(1) {
    if (RingBuffer_IsFull(&USBtoUSART_Buffer))
      break;
    
    int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial2_CDC_Interface);
    if (ReceivedByte < 0)
      break;
    
    /* Store received byte into the USART transmit buffer */
    RingBuffer_Insert(&USBtoUSART_Buffer, ReceivedByte);
    USART_DreInterruptLevel_Set(&USART, USART_DREINTLVL_LO_gc);
  }
  
  // PIGATOR => USB
  uint16_t BufferCount = RingBuffer_GetCount(&USARTtoUSB_Buffer);
  if (BufferCount) {
    Endpoint_SelectEndpoint(VirtualSerial2_CDC_Interface.Config.DataINEndpoint.Address);
    
    /* Check if a packet is already enqueued to the host - if so, we shouldn't try to send more data
     * until it completes as there is a chance nothing is listening and a lengthy timeout could occur */
    if (Endpoint_IsINReady()) {
      /* Never send more than one bank size less one byte to the host at a time, so that we don't block
       * while a Zero Length Packet (ZLP) to terminate the transfer is sent if the host isn't listening */

      uint8_t BytesToSend = MIN(BufferCount, (CDC_TXRX_EPSIZE - 1));
      
      /* Read bytes from the USART receive buffer into the USB IN endpoint */
      while (BytesToSend--) {

	/* Try to send the next byte of data to the host, abort if there is an error without dequeuing */
	if (CDC_Device_SendByte(&VirtualSerial2_CDC_Interface, RingBuffer_Peek(&USARTtoUSB_Buffer)) != ENDPOINT_READYWAIT_NoError)
	  break;
	
	/* Dequeue the already sent byte from the buffer now we have confirmed that no transmission error occurred */
	RingBuffer_Remove(&USARTtoUSB_Buffer);
      }
    }
  }
}

Pigator_Module_t modules[] = {

  {
    .Magic           = "CSM",
    .Baud            = 38400,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = pig_reset_low,
    .cb_mod_bootload = pig_csm_bootload,
  },

  {
    .Magic           = "TPUART",
    .Baud            = 19200,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = NULL,
    .cb_mod_bootload = NULL,
  },

  {
    .Magic           = "ZM3102",
    .Baud            = 115200,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = pig_reset_low,
    .cb_mod_bootload = NULL,
  },

  {
    .Magic           = "TCM310",
    .Baud            = 57600,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = pig_reset_high,
    .cb_mod_bootload = NULL,
  },

  { .Magic        = "\x00" } // EOL
};


void pigator_init(void) {

  Serial_Disable(&USART);
  
  TWI_Init(&TWIE, TWI_BAUD_FROM_FREQ(100000));
  
  // Enable internal pull-up on PC0, PC1.. Uncomment if you don't have external pullups
  PORTCFG.MPCMASK = 0x03; // Configure several PINxCTRL registers at the same time
  PORTE.PIN0CTRL = (PORTE.PIN0CTRL & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc; //Enable pull-up to get a defined level on the switches

  memset(EEP_MAGIC, 0, sizeof(EEP_MAGIC));
  Pigator_Module   = NULL;
  pig_mod_init     = NULL;
  pig_mod_task     = NULL;
  pig_mod_reset    = NULL;
  pig_mod_bootload = NULL;

  const uint8_t EEPAddress = 0;
  if (TWI_ReadPacket(&TWIE, EEP_ADDR, 10, &EEPAddress, sizeof(EEPAddress), (uint8_t *)EEP_MAGIC, sizeof(EEP_MAGIC)-1) == TWI_ERROR_NoError) {

    // Find Magic in available modules
    for(uint8_t idx = 0; ; idx++) {

      if (strlen(modules[idx].Magic)==0)
	break;

      if (!strncmp(EEP_MAGIC, modules[idx].Magic, strlen(modules[idx].Magic))) {
	Pigator_Module = &modules[idx];
	break;
      }
      
    }

  } else {

    memset(EEP_MAGIC, 0, sizeof(EEP_MAGIC));
    return;
  }

  if (!Pigator_Module)
    return;
  
  pig_mod_init     = Pigator_Module->cb_mod_init;
  pig_mod_task     = Pigator_Module->cb_mod_task;
  pig_mod_reset    = Pigator_Module->cb_mod_reset;
  pig_mod_bootload = Pigator_Module->cb_mod_bootload;

  if (pig_mod_init)
    pig_mod_init();

}

void pigator_task(void) {
  if (pig_mod_task)
    pig_mod_task();
}

void pigator_func(char *in) {
  if (in[1] == 'i') {                // Info
    DS_P(PSTR("1:"));
    if (Pigator_Module) {
      DS( Pigator_Module->Magic );
    } else {
      DS( (char *)EEP_MAGIC );
      DS_P(PSTR(" ?"));
    }
    DNL();
  } else if (in[1] == 'r') {  // reset module
    reset_module();
  } else if (in[1] == 'b') {  // call modules bootloader
    if (pig_mod_bootload)
      pig_mod_bootload();
  }
}

ISR(USARTD0_RXC_vect) {
  if (!(RingBuffer_IsFull(&USARTtoUSB_Buffer)))
    RingBuffer_Insert(&USARTtoUSB_Buffer, Serial_ReceiveByte(&USART));
}

ISR(USARTD0_DRE_vect) {

  /* Load the next byte from the USART transmit buffer into the USART if transmit buffer space is available 
     or disable interrupt */
  if ((RingBuffer_IsEmpty(&USBtoUSART_Buffer))) {
    USART_DreInterruptLevel_Set(&USART, USART_DREINTLVL_OFF_gc);
  } else 
    Serial_SendByte(&USART, RingBuffer_Remove(&USBtoUSART_Buffer));
}

#endif

