#include <lyos/const.h>
#include <lyos/sysutils.h>
#include <lyos/usb.h>

#include "usb.h"

static DEF_LIST(driver_list);

int usb_match_device(struct usb_device* dev, const struct usb_device_id* id)
{
    if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
        id->idVendor != le16_to_cpu(dev->descriptor.idVendor))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
        id->idProduct != le16_to_cpu(dev->descriptor.idProduct))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
        (id->bcdDevice_lo > le16_to_cpu(dev->descriptor.bcdDevice)))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
        (id->bcdDevice_hi < le16_to_cpu(dev->descriptor.bcdDevice)))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
        (id->bDeviceClass != dev->descriptor.bDeviceClass))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
        (id->bDeviceSubClass != dev->descriptor.bDeviceSubClass))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
        (id->bDeviceProtocol != dev->descriptor.bDeviceProtocol))
        return FALSE;

    return TRUE;
}

int usb_match_one_id_intf(struct usb_device* dev,
                          struct usb_host_interface* intf,
                          const struct usb_device_id* id)
{
    if (dev->descriptor.bDeviceClass == USB_CLASS_VENDOR_SPEC &&
        !(id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
        (id->match_flags &
         (USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS |
          USB_DEVICE_ID_MATCH_INT_PROTOCOL | USB_DEVICE_ID_MATCH_INT_NUMBER)))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
        (id->bInterfaceClass != intf->desc.bInterfaceClass))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
        (id->bInterfaceSubClass != intf->desc.bInterfaceSubClass))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
        (id->bInterfaceProtocol != intf->desc.bInterfaceProtocol))
        return FALSE;

    if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_NUMBER) &&
        (id->bInterfaceNumber != intf->desc.bInterfaceNumber))
        return FALSE;

    return TRUE;
}

int usb_match_one_id(struct usb_interface* interface,
                     const struct usb_device_id* id)
{
    struct usb_host_interface* intf;
    struct usb_device* dev;

    if (id == NULL) return 0;

    intf = interface->cur_altsetting;
    dev = interface->parent;

    if (!usb_match_device(dev, id)) return 0;

    return usb_match_one_id_intf(dev, intf, id);
}

const struct usb_device_id* usb_match_id(struct usb_interface* interface,
                                         const struct usb_device_id* id)
{
    if (id == NULL) return NULL;

    for (; id->idVendor || id->idProduct || id->bDeviceClass ||
           id->bInterfaceClass;
         id++) {
        if (usb_match_one_id(interface, id)) return id;
    }

    return NULL;
}

const struct usb_device_id* usb_device_match_id(struct usb_device* udev,
                                                const struct usb_device_id* id)
{
    if (!id) return NULL;

    for (; id->idVendor || id->idProduct; id++) {
        if (usb_match_device(udev, id)) return id;
    }

    return NULL;
}

int usb_register_driver(struct usb_driver* driver)
{
    list_add(&driver->list, &driver_list);
    return 0;
}

void usb_probe_interface(struct usb_interface* intf)
{
    struct usb_driver* drv;
    const struct usb_device_id* id;
    int retval;

    list_for_each_entry(drv, &driver_list, list)
    {
        id = usb_match_id(intf, drv->id_table);
        if (id) {
            retval = drv->probe(intf, id);
            if (retval == 0) break;
        }
    }
}
