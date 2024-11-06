#
# Makefile for the Lyos HID device drivers

SUBDIRS	=

ifeq ($(CONFIG_KEYBOARD_ATKBD),y)
	SUBDIRS += atkbd
endif

ifeq ($(CONFIG_USB_HID),y)
	SUBDIRS += usbhid
endif

include lyos.subdirs.mk
