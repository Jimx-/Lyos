#ifndef _USBD_PROTO_H_
#define _USBD_PROTO_H_

#if CONFIG_USB_DWC2
void dwc2_scan(void);
#endif

int usb_hub_init(void);

#endif
