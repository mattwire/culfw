#include "board.h"
#ifdef HAS_USB
#include "led.h"
#include "ringbuffer.h"
#include "usb.h"
#include "ttydata.h"
#include "display.h"
#include "pigator.h"

USB_ClassInfo_CDC_Device_t VirtualSerial1_CDC_Interface = {
  .Config =
  {
    .ControlInterfaceNumber   = INTERFACE_ID_CDC1_CCI,
    .DataINEndpoint           =
    {
      .Address          = CDC1_TX_EPADDR,
      .Size             = CDC_TXRX_EPSIZE,
      .Banks            = 1,
    },
    .DataOUTEndpoint =
    {
      .Address          = CDC1_RX_EPADDR,
      .Size             = CDC_TXRX_EPSIZE,
      .Banks            = 1,
    },
    .NotificationEndpoint =
    {
      .Address          = CDC1_NOTIFICATION_EPADDR,
      .Size             = CDC_NOTIFICATION_EPSIZE,
      .Banks            = 1,
    },
  },
};

USB_ClassInfo_CDC_Device_t VirtualSerial2_CDC_Interface = {
  .Config =
  {
    .ControlInterfaceNumber   = INTERFACE_ID_CDC2_CCI,
    .DataINEndpoint           =
    {
      .Address          = CDC2_TX_EPADDR,
      .Size             = CDC_TXRX_EPSIZE,
      .Banks            = 1,
    },
    .DataOUTEndpoint =
    {
      .Address          = CDC2_RX_EPADDR,
      .Size             = CDC_TXRX_EPSIZE,
      .Banks            = 1,
    },
    .NotificationEndpoint =
    {
      .Address          = CDC2_NOTIFICATION_EPADDR,
      .Size             = CDC_NOTIFICATION_EPSIZE,
      .Banks            = 1,
    },
    
  },
};

void
usb_init(void) {
  USB_Init();

}

void
usb_task(void){
  
  usb_rxtx();

  CDC_Device_USBTask(&VirtualSerial1_CDC_Interface);
  CDC_Device_USBTask(&VirtualSerial2_CDC_Interface);
  USB_USBTask();
}

void
usb_rxtx(void)
{

  // receive from host
  // able to take new data?
  while (TTY_Rx_Buffer.nbytes < TTY_BUFSIZE) {
    int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial1_CDC_Interface);
    if (ReceivedByte < 0) 
      break;
    
    rb_put(&TTY_Rx_Buffer, ReceivedByte);
  }
  
  output_flush_func = usb_rxtx;
  input_handle_func(DISPLAY_USB);
    
  // send to host
  Endpoint_SelectEndpoint(VirtualSerial1_CDC_Interface.Config.DataINEndpoint.Address);
  if (TTY_Tx_Buffer.nbytes && Endpoint_IsINReady()) {
    uint8_t maxbytes = CDC_TXRX_EPSIZE;
    while (TTY_Tx_Buffer.nbytes && maxbytes-->0) {
      CDC_Device_SendByte(&VirtualSerial1_CDC_Interface, rb_get(&TTY_Tx_Buffer));
    }
  }
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  bool ConfigSuccess = true;
  
  ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial1_CDC_Interface);
  ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial2_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
  CDC_Device_ProcessControlRequest(&VirtualSerial1_CDC_Interface);
  CDC_Device_ProcessControlRequest(&VirtualSerial2_CDC_Interface);
}


void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo) {
  uint8_t ConfigMask = 0;

  // only PIM interface might change baudrate
  if (CDCInterfaceInfo->Config.ControlInterfaceNumber == INTERFACE_ID_CDC2_CCI) {
    
    switch (CDCInterfaceInfo->State.LineEncoding.ParityType) {
    case CDC_PARITY_Odd:
      ConfigMask |= USART_PMODE_ODD_gc;
      break;
    case CDC_PARITY_Even:
      ConfigMask |= USART_PMODE_EVEN_gc;
      break;
    }
    
    if (CDCInterfaceInfo->State.LineEncoding.CharFormat == CDC_LINEENCODING_TwoStopBits)
      ConfigMask |= 0x8;
    
    switch (CDCInterfaceInfo->State.LineEncoding.DataBits) {
    case 5:
      ConfigMask |= USART_CHSIZE_5BIT_gc;
      break;
    case 6:
      ConfigMask |= USART_CHSIZE_6BIT_gc;
      break;
    case 7:
      ConfigMask |= USART_CHSIZE_7BIT_gc;
      break;
    case 8:
      ConfigMask |= USART_CHSIZE_8BIT_gc;
      break;
    case 9:
      ConfigMask |= USART_CHSIZE_9BIT_gc;
      break;
    }
    
    PIM_setBaud( CDCInterfaceInfo->State.LineEncoding.BaudRateBPS, ConfigMask );
    
  }
  
}


#endif