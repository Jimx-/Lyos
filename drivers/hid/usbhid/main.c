#include <lyos/types.h>
#include <lyos/ipc.h>
#include <lyos/const.h>
#include <errno.h>
#include <unistd.h>
#include <lyos/service.h>
#include <lyos/sysutils.h>
#include <lyos/input.h>
#include <libinputdriver/libinputdriver.h>
#include <asm/const.h>

#include <uapi/linux/input-event-codes.h>

#define NAME "usbhid"

static void usbhid_interrupt(unsigned long irq_set);

static struct inputdriver usbhid_driver = {
    .input_interrupt = usbhid_interrupt,
};

static void usbhid_interrupt(unsigned long irq_set) {}

static int usbhid_init(void)
{
    printl(NAME ": USB HID driver is running\n");

    return 0;
}

int main()
{
    serv_register_init_fresh_callback(usbhid_init);
    serv_init();

    return inputdriver_start(&usbhid_driver);
}
