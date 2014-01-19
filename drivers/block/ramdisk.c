/*  This file is part of Lyos.

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
    
#include "lyos/type.h"
#include "sys/types.h"
#include "stdio.h"
#include "unistd.h"
#include "lyos/config.h"
#include "lyos/const.h"
#include "string.h"
#include "lyos/fs.h"
#include "lyos/proc.h"
#include "lyos/tty.h"
#include "lyos/console.h"
#include "lyos/global.h"
#include "lyos/proto.h"

#define MAX_RAMDISKS	8

struct ramdisk_dev {
	unsigned char * start;
	int length;

	int rdonly;
};

struct ramdisk_dev ramdisks[MAX_RAMDISKS];

PRIVATE void rd_rdwt(MESSAGE * p);

PUBLIC void task_rd()
{
	MESSAGE msg;

	while(!rd_base || !rd_length);
	init_rd();

	while (1) {
	
		send_recv(RECEIVE, ANY, &msg);

		int src = msg.source;
		switch (msg.type){
		case DEV_OPEN:
			break;
		case DEV_CLOSE:
			break;
		case DEV_READ:
		case DEV_WRITE:
			rd_rdwt(&msg);
			break;
		case DEV_IOCTL:
			break;
		default:
			dump_msg("ramdisk driver: unknown msg", &msg);
			spin("FS::main_loop (invalid msg.type)");
			break;
		}

		send_recv(SEND, src, &msg);
	}
}

PRIVATE void rd_rdwt(MESSAGE * p)
{
	u64 pos = p->POSITION;
	int count = p->CNT;
	int dev = MINOR(p->DEVICE);
	struct ramdisk_dev * ramdisk = ramdisks + dev;
	char * addr = ramdisk->start + pos;
	
	if (pos > ramdisk->length){
		p->CNT = 0;
		return;
	}

	if (p->type == DEV_WRITE){
		data_copy(getpid(), D, addr, p->source, D, p->BUF, count);
	}else if(p->type == DEV_READ){
		data_copy(p->source, D, p->BUF, getpid(), D, addr, count);
	}
}

PUBLIC void init_rd()
{
	printl("RAMDISK: initrd: %d bytes(%d kB), base: 0x%x\n", rd_length, rd_length / 1024, rd_base);
	struct ramdisk_dev * initrd = ramdisks;

	initrd->start = rd_base;
	initrd->length = rd_length;
	initrd->rdonly = 1;
}
