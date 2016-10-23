#include "board.h"
#ifdef HAS_PIGATOR

#include "pigator.h"
#include "registry.h"
#include "LUFA/Drivers/Peripheral/TWI.h"
#include <LUFA/Drivers/Peripheral/Serial.h>
#include <LUFA/Drivers/Misc/RingBuffer.h>

#ifdef HAS_USB
#include "Descriptors.h"
#include "usb.h"
#endif

#include "arch.h"
#include "string.h"
#include "display.h"
#include "led.h"
#include <avr/pgmspace.h>

#ifdef HAS_ESP8266
#include "esp8266.h"
#endif

#ifdef HAS_W5500
#include "ethernet.h"
#endif

#define USART PIG_UART
#define TWI   PIG_TWI

RingBuffer_t toPIM_Buffer;
uint8_t      toPIM_Buffer_Data[USART_BUF_SIZE];
RingBuffer_t PIMtoUSB_Buffer;
uint8_t      PIMtoUSB_Buffer_Data[USART_BUF_SIZE];

Pigator_Module_t *Pigator_Module = NULL;

#define EEP_ADDR  0xA0
#ifdef PIG_FIXED
char EEP_MAGIC[10] = PIG_FIXED;
#else
static char EEP_MAGIC[10];
#endif

#ifdef PIG_STACKED
static char mycmd[256];
static uint8_t mypos = 0;
static uint8_t myEOL = 0;
#endif

void(* pig_mod_task)(void) = NULL;
void(* pig_mod_init)(void) = NULL;
void(* pig_mod_reset)(uint8_t on) = NULL;
void(* pig_mod_bootload)(void) = NULL;

void pig_reset_low(uint8_t on) {

  PIG_RESET_PORT.DIRSET = PIG_RESET_PIN;

  if (on) {
    PIG_RESET_PORT.OUTCLR = PIG_RESET_PIN;
  } else
    PIG_RESET_PORT.OUTSET = PIG_RESET_PIN;
  
}

void pig_reset_high(uint8_t on) {

  PIG_RESET_PORT.DIRSET = PIG_RESET_PIN;

  if (on) {
    PIG_RESET_PORT.OUTSET = PIG_RESET_PIN;
  } else
    PIG_RESET_PORT.OUTCLR = PIG_RESET_PIN;
  
}

void reset_module(void) {
  if (pig_mod_reset) {
    pig_mod_reset( 1 );
    my_delay_ms(10);
    pig_mod_reset( 0 );
    my_delay_ms(10);
  }
}

void pig_bsel_low_bootload(void) {
  PIG_BSEL_PORT.DIRSET = PIG_BSEL_PIN;
  pig_mod_reset( 1 );

  PIG_BSEL_PORT.OUTCLR = PIG_BSEL_PIN;
  my_delay_ms(10);
  pig_mod_reset( 0 );
  my_delay_ms(500);
  PIG_BSEL_PORT.OUTSET = PIG_BSEL_PIN;
  PIG_BSEL_PORT.DIRCLR = PIG_BSEL_PIN;
}

void pig_serialfwd_init(void) {

  if (!Pigator_Module)
    return;

  // USART RX/TX 1
  /* PIN3 (TXD0) as output. */
  PIG_UART_PORT.DIRSET = PIN3_bm;
  /* PIN2 (RXD0) as input. */
  PIG_UART_PORT.DIRCLR = PIN2_bm;
  
  mySerial_Init(&USART, Pigator_Module->Baud, Pigator_Module->Coding);
  USART_RxdInterruptLevel_Set(&USART, USART_RXCINTLVL_LO_gc);

  RingBuffer_InitBuffer(&toPIM_Buffer, toPIM_Buffer_Data, USART_BUF_SIZE);
  RingBuffer_InitBuffer(&PIMtoUSB_Buffer, PIMtoUSB_Buffer_Data, USART_BUF_SIZE);

  reset_module();
}

void toPIMBuffer(uint8_t in) {
  if (RingBuffer_IsFull(&toPIM_Buffer))
    return;
  
  RingBuffer_Insert(&toPIM_Buffer, in);
  USART_DreInterruptLevel_Set(&USART, USART_DREINTLVL_LO_gc);
}

#ifdef PIG_STACKED
void pig_stacker_task(void) {
  uint8_t data;

  // PIGATOR => Host
  uint16_t BufferCount = RingBuffer_GetCount(&PIMtoUSB_Buffer);
  if (BufferCount) {
    
    while (BufferCount--) {
      data = RingBuffer_Remove(&PIMtoUSB_Buffer);
      
      // EOL ?
      if (data == 13 || data == 10) {
	
	if (mypos) {
	  mycmd[mypos] = 0;
	  
	  DC( '*' );
	  DS( mycmd );
	  DNL();
	}
	
	mypos = 0;
	mycmd[mypos] = 0;
	
      } else 
	mycmd[mypos++] = data;
      
    }
  }
  
}
#endif

void pig_serialfwd_task(void) {

#ifdef HAS_USB

  // USB => PIGATOR
  while(USB_IsConnected) {
    if (RingBuffer_IsFull(&toPIM_Buffer))
      break;
    
    int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial2_CDC_Interface);
    if (ReceivedByte < 0)
      break;

    /* Store received byte into the USART transmit buffer */
    toPIMBuffer( ReceivedByte );
  }

  // PIGATOR => USB
  uint16_t BufferCount = RingBuffer_GetCount(&PIMtoUSB_Buffer);
  if (USB_IsConnected && BufferCount) {
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
	if (CDC_Device_SendByte(&VirtualSerial2_CDC_Interface, RingBuffer_Peek(&PIMtoUSB_Buffer)) != ENDPOINT_READYWAIT_NoError)
	  break;
	
	/* Dequeue the already sent byte from the buffer now we have confirmed that no transmission error occurred */
	RingBuffer_Remove(&PIMtoUSB_Buffer);
      }
    }
  }

#endif

}

Pigator_Module_t modules[] = {

#ifdef PIG_STACKED

  {
    .Magic           = "CSM",
    .Baud            = 38400,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_stacker_task,
    .cb_mod_reset    = pig_reset_low,
    .cb_mod_bootload = NULL,
  },

#else
  
  {
    .Magic           = "CSM",
    .Baud            = 38400,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = pig_reset_low,
    .cb_mod_bootload = pig_bsel_low_bootload,
  },

  {
    .Magic           = "S0X2",
    .Baud            = 38400,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = pig_reset_low,
    .cb_mod_bootload = pig_bsel_low_bootload,
  },

  {
    .Magic           = "ESP8266",
    .Baud            = 9600,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = pig_reset_low,
    .cb_mod_bootload = pig_bsel_low_bootload,
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
    .Magic           = "DS2480",
    .Baud            = 9600,
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

  {
    .Magic           = "RS485",
    .Baud            = 9600,
    .hasRTS          = 1,
    .flexBaud        = 1,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = NULL,
    .cb_mod_bootload = NULL,
  },

  {
    .Magic           = "SERIAL",
    .Baud            = 9600,
    .flexBaud        = 1,
    .cb_mod_init     = pig_serialfwd_init,
    .cb_mod_task     = pig_serialfwd_task,
    .cb_mod_reset    = NULL,
    .cb_mod_bootload = NULL,
  },

#endif
  
  { .Magic        = "\x00" } // EOL
};


void pigator_init(void) {

  Serial_Disable(&USART);
  
  Pigator_Module   = NULL;
  pig_mod_init     = NULL;
  pig_mod_task     = NULL;
  pig_mod_reset    = NULL;
  pig_mod_bootload = NULL;

#ifdef PIG_FIXED
  if (1) {
#else
  
  memset(EEP_MAGIC, 0, sizeof(EEP_MAGIC));

  TWI_Init(&TWI, TWI_BAUD_FROM_FREQ(100000));
  TWI.CTRL |= TWI_SDAHOLD_50NS_gc; // Onewire PIM w/ level changers requires this

#ifdef PIG_I2C_PULL
  // Enable internal pull-up on PIN0, PIN1..
  PORTCFG.MPCMASK = 0x03; // Configure several PINxCTRL registers at the same time
  PIG_I2C_PULL.PIN0CTRL = (PIG_I2C_PULL.PIN0CTRL & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc; //Enable pull-up to get a defined level on the switches
#endif  

  const uint8_t EEPAddress = 0;
  if (TWI_ReadPacket(&TWI, EEP_ADDR, 10, &EEPAddress, sizeof(EEPAddress), (uint8_t *)EEP_MAGIC, sizeof(EEP_MAGIC)-1) == TWI_ERROR_NoError) {

#endif    

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

#ifdef PIG_RTS_PORT
  if (Pigator_Module->hasRTS) {
    PIG_RTS_PORT.DIRSET = PIG_RTS_PIN;
    PIG_RTS_PORT.OUTCLR = PIG_RTS_PIN;
  };
#endif

  Pigator_Module->Coding = USART_CHSIZE_8BIT_gc; // 8N1
  
  // if baudrate may be flexable - read last given value from registry
  if (Pigator_Module->flexBaud) {
    registry_get( REG_PIM_BAUD, &(Pigator_Module->Baud));
    registry_get( REG_PIM_FORMAT, &(Pigator_Module->Coding));
  } 

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
      if (Pigator_Module->flexBaud) {
	DS_P(PSTR(" @ "));
	DU(Pigator_Module->Baud,0); DC('x');
	DH2(USART.CTRLC);
      }
    } else {
      DS( (char *)EEP_MAGIC );
      DS_P(PSTR(" ?"));
    }
    DNL();
  } else if (in[1] == 'r') {  // reset module
    reset_module();
  } else if (in[1] == 's') {  // store baudrate
    if (Pigator_Module && Pigator_Module->Baud)
      registry_set( REG_PIM_BAUD, &(Pigator_Module->Baud), 4);
      registry_set( REG_PIM_FORMAT, &(Pigator_Module->Coding), 1);
  } else if (in[1] == 'b') {  // call modules bootloader
    if (pig_mod_bootload)
      pig_mod_bootload();
  }
}

void pigator_stack_func(char *in) {
  // downlink message to PIM by removing one '*'
  uint8_t i = 1;

  while (in[i]) { 
    RingBuffer_Insert(&toPIM_Buffer, in[i++]);
  }

  RingBuffer_Insert(&toPIM_Buffer, 13); // add CR
  RingBuffer_Insert(&toPIM_Buffer, 10); // add NL
  
  USART_DreInterruptLevel_Set(&USART, USART_DREINTLVL_LO_gc);
}

ISR(PIG_RXC_vect) {

  USART.STATUS = USART_RXCIF_bm;

  uint8_t data = USART.DATA;
  
  if (!(RingBuffer_IsFull(&PIMtoUSB_Buffer)))
    RingBuffer_Insert(&PIMtoUSB_Buffer, data);

#ifdef HAS_ESP8266
  rb_put(&ESP_Rx_Buffer, data);
#endif

#ifdef HAS_W5500
  toNETBuffer(data);
#endif
  
}

ISR(PIG_DRE_vect) {

  /* Load the next byte from the USART transmit buffer into the USART if transmit buffer space is available 
     or disable interrupt */
  
  if ((RingBuffer_IsEmpty(&toPIM_Buffer))) {
    USART_DreInterruptLevel_Set(&USART, USART_DREINTLVL_OFF_gc);
    
    if (Pigator_Module && Pigator_Module->hasRTS)
      USART_TxdInterruptLevel_Set(&USART, USART_TXCINTLVL_LO_gc);
    
  } else {
    
    if (Pigator_Module && Pigator_Module->hasRTS) {
      USART.CTRLB &= ~USART_RXEN_bm;
      PIG_RTS_PORT.OUTSET = PIG_RTS_PIN;
    }
    
    Serial_SendByte(&USART, RingBuffer_Remove(&toPIM_Buffer));
    
  }
}

ISR(PIG_TXC_vect) {
  USART_TxdInterruptLevel_Set(&USART, USART_TXCINTLVL_OFF_gc);
  if ((Pigator_Module && Pigator_Module->hasRTS) && RingBuffer_IsEmpty(&toPIM_Buffer))
    PIG_RTS_PORT.OUTCLR = PIG_RTS_PIN;

  USART.CTRLB |= USART_RXEN_bm;
}

void PIM_setBaud(uint32_t baud, uint8_t format) {
  if (baud && Pigator_Module && Pigator_Module->flexBaud) {

    if (baud != Pigator_Module->Baud || Pigator_Module->Coding != format) {
      Pigator_Module->Baud = baud;
      Pigator_Module->Coding = format;

      //      DS_P(PSTR("Setting PIM to ")); DU(baud,0); DS_P(PSTR(" baud\n\r"));
      //      DS_P(PSTR("Setting PIM to ")); DH2(format); DS_P(PSTR(" coding\n\r"));
      //      registry_set( REG_PIM_BAUD, &baud, 4);
      
      if (pig_mod_init)
	pig_mod_init();
      
    }
    
  }
}

#endif

