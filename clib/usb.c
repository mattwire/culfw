#include "led.h"
#include "ringbuffer.h"
#include "usb.h"
#include "ttydata.h"
#include "display.h"

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
  {
    .Config =
    {
      .ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
      .DataINEndpoint           =
      {
	.Address          = CDC_TX_EPADDR,
	.Size             = CDC_TXRX_EPSIZE,
	.Banks            = 1,
      },
      .DataOUTEndpoint =
      {
	.Address          = CDC_RX_EPADDR,
	.Size             = CDC_TXRX_EPSIZE,
	.Banks            = 1,
      },
      .NotificationEndpoint =
      {
	.Address          = CDC_NOTIFICATION_EPADDR,
	.Size             = CDC_NOTIFICATION_EPSIZE,
	.Banks            = 1,
      },
    },
  };

void
usb_init(void) {
  USB_Init();

  GlobalInterruptEnable();
}

void
usb_task(void){
  
  usb_rxtx();

  CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
  USB_USBTask();
}

void
usb_rxtx(void)
{
  // receive from host
  // able to take new data?
  while (TTY_Rx_Buffer.nbytes < TTY_BUFSIZE) {
    int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    if (ReceivedByte < 0) 
      break;
    
    rb_put(&TTY_Rx_Buffer, ReceivedByte);
  }
  
  output_flush_func = usb_rxtx;
  input_handle_func(DISPLAY_USB);
    
  // send to host
  Endpoint_SelectEndpoint(VirtualSerial_CDC_Interface.Config.DataINEndpoint.Address);
  if (TTY_Tx_Buffer.nbytes && Endpoint_IsINReady()) {
    uint8_t maxbytes = CDC_TXRX_EPSIZE;
    while (TTY_Tx_Buffer.nbytes && maxbytes-->0) {
      CDC_Device_SendByte(&VirtualSerial_CDC_Interface, rb_get(&TTY_Tx_Buffer));
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

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}


/*
void
usb_flush(void){
}

////////////////////
// Fill data from USB to the RingBuffer and vice-versa
void
CDC_Task(void)
{
  static char inCDC_TASK = 0;

  if(!USB_IsConnected)
    return;

  Endpoint_SelectEndpoint(CDC_RX_EPNUM);          // First data in
  if(!inCDC_TASK && Endpoint_IsReadWriteAllowed()){ // USB -> RingBuffer

    while (Endpoint_BytesInEndpoint()) {          // Discard data on buffer full
      rb_put(&TTY_Rx_Buffer, Endpoint_Read_Byte());
    }
    Endpoint_ClearOUT(); 

    inCDC_TASK = 1;
    output_flush_func = CDC_Task;
    input_handle_func(DISPLAY_USB);
    inCDC_TASK = 0;
  }


  Endpoint_SelectEndpoint(CDC_TX_EPNUM);          // Then data out
  if(TTY_Tx_Buffer.nbytes && Endpoint_IsReadWriteAllowed()) {

    cli();
    while(TTY_Tx_Buffer.nbytes &&
          (Endpoint_BytesInEndpoint() < USB_BUFSIZE))
      Endpoint_Write_Byte(rb_get(&TTY_Tx_Buffer));
    sei();
    
    bool IsFull = (Endpoint_BytesInEndpoint() == USB_BUFSIZE);
    Endpoint_ClearIN();                  // Send the data
    if(IsFull) {
      Endpoint_WaitUntilReady();
      Endpoint_ClearIN();
    }
  }
}

void
cdc_flush()
{
  Endpoint_SelectEndpoint(CDC_TX_EPNUM);
  Endpoint_WaitUntilReady();
  Endpoint_ClearIN();
}

*/
