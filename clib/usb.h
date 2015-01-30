#ifndef _USB_H_
#define _USB_H_

#include "Descriptors.h"
#include <LUFA/Drivers/USB/USB.h>

void usb_init(void);
void usb_task(void);
void usb_rxtx(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

#define USB_IsConnected (USB_DeviceState == DEVICE_STATE_Configured)

#endif
