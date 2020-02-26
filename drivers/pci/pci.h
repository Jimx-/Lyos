/*  (c)Copyright 2011 Jimx

    This file is part of Lyos.

    Lyos is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lyos is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lyos.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _PCI_H_
#define _PCI_H_

#include <libdevman/libdevman.h>

struct pcidev {
    u8 busnr;
    u8 dev;
    u8 func;

    u16 vid;
    u16 did;

    u8 baseclass;
    u8 subclass;
    u8 infclass;

    device_id_t dev_id;
};

struct pcibus {
    int busnr;
    device_id_t dev_id;

    u8 (*rreg_u8)(u32 busind, u32 devind, u16 port);
    u16 (*rreg_u16)(u32 busind, u32 devind, u16 port);
    u32 (*rreg_u32)(u32 busind, u32 devind, u16 port);
    void (*wreg_u16)(u32 busind, u32 devind, u16 port, u16 value);
};

struct pci_device {
    int vendor;
    int device_id;
    char* name;
};

extern struct pcibus pcibus[];
extern struct pcidev pcidev[];
extern struct pci_acl pci_acl[];

PUBLIC char* pci_dev_name(int vendor, int device_id);

PUBLIC u8 pcii_read_u8(u32 bus, u32 slot, u32 func, u16 offset);
PUBLIC u16 pcii_read_u16(u32 bus, u32 slot, u32 func, u16 offset);
PUBLIC u32 pcii_read_u32(u32 bus, u32 slot, u32 func, u16 offset);
PUBLIC void pcii_write_u16(u32 bus, u32 slot, u32 func, u16 offset, u16 value);
PUBLIC u8 pcii_rreg_u8(u32 busind, u32 devind, u16 port);
PUBLIC u16 pcii_rreg_u16(u32 busind, u32 devind, u16 port);
PUBLIC u32 pcii_rreg_u32(u32 busind, u32 devind, u16 port);
PUBLIC void pcii_wreg_u16(u32 busind, u32 devind, u16 port, u16 value);

PUBLIC int _pci_first_dev(struct pci_acl* acl, int* devind, u16* vid, u16* did,
                          device_id_t* dev_id);
PUBLIC int _pci_next_dev(struct pci_acl* acl, int* devind, u16* vid, u16* did,
                         device_id_t* dev_id);

PUBLIC u8 pci_read_attr_u8(int devind, int port);
PUBLIC u16 pci_read_attr_u16(int devind, int port);
PUBLIC u32 pci_read_attr_u32(int devind, int port);
PUBLIC void pci_write_attr_u16(int devind, int port, u16 value);

#endif
