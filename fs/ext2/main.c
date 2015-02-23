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
#include "lyos/config.h"
#include "errno.h"
#include "stdio.h"
#include "stddef.h"
#include "unistd.h"
#include "assert.h"
#include "lyos/const.h"
#include "string.h"
#include "lyos/fs.h"
#include "lyos/proc.h"
#include "lyos/global.h"
#include "lyos/proto.h"
#include "lyos/list.h"
#include "libfsdriver/libfsdriver.h"
#include "ext2_fs.h"
#include "global.h"

#define DEBUG
#ifdef DEBUG
#define DEB(x) printl("ext2fs: "); x
#else
#define DEB(x)
#endif

#define BLOCK2SECTOR(blk_size, blk_nr) (blk_nr * (blk_size))

PUBLIC int init_ext2fs();

PRIVATE int ext2_mountpoint(MESSAGE * p);

struct fsdriver ext2fsdriver = {
    .name = "ext2",

    .fs_readsuper = ext2_readsuper,
    .fs_putinode = ext2_putinode,
    .fs_create = ext2_create,
    .fs_stat = ext2_stat,
    .fs_ftrunc = ext2_ftrunc,
    .fs_sync = ext2_sync,
};

/*****************************************************************************
 *                                task_ext2_fs
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK Ext2 FS.
 * 
 *****************************************************************************/
PUBLIC int main()
{
    serv_register_init_fresh_callback(init_ext2fs);
    serv_init();

	fsdriver_start(&ext2fsdriver);

	int reply;

    MESSAGE m;
	while (1) {
		send_recv(RECEIVE, ANY, &m);

		int msgtype = m.type;
		int src = m.source;
		reply = 1;
		switch (msgtype) {
		case FS_LOOKUP:
			m.RET_RETVAL = ext2_lookup(&m);
            break;
		case FS_PUTINODE:
            m.RET_RETVAL = fsdriver_putinode(&ext2fsdriver, &m);
			break;
        case FS_MOUNTPOINT:
            m.RET_RETVAL = ext2_mountpoint(&m);
            break;
        case FS_READSUPER:
            m.RET_RETVAL = fsdriver_readsuper(&ext2fsdriver, &m);
            break;
        case FS_STAT:
        	m.STRET = fsdriver_stat(&ext2fsdriver, &m);
        	break;
        case FS_RDWT:
        	m.RWRET = ext2_rdwt(&m);
        	break;
        case FS_CREATE:
        	m.CRRET = fsdriver_create(&ext2fsdriver, &m);
        	break;
        case FS_FTRUNC:
        	m.RET_RETVAL = fs_ftrunc(&ext2fsdriver, &m);
        	break;
        case FS_SYNC:
        	m.RET_RETVAL = fs_sync(&ext2fsdriver, &m);
        	break;
		default:
			m.RET_RETVAL = ENOSYS;
			break;
		}

		/* reply */
		if (reply) {
			m.type = FSREQ_RET;
			send_recv(SEND, src, &m);
		}

        ext2_sync();
	}

    return 0;
}

PUBLIC int init_ext2fs()
{
    printl("ext2fs: Ext2 filesystem driver is running\n");
    ext2_ep = get_endpoint();
    
	ext2_init_inode();
    ext2_init_buffer_cache();

    err_code = 0;

    return 0;
}

PRIVATE int ext2_mountpoint(MESSAGE * p)
{
    dev_t dev = p->REQ_DEV;
    ino_t num = p->REQ_NUM;

    ext2_inode_t * pin = get_ext2_inode(dev, num);

    if (pin == NULL) return EINVAL;

    if (pin->i_mountpoint) return EBUSY;

    int retval = 0;
    mode_t bits = pin->i_mode & I_TYPE;
    if (bits == I_BLOCK_SPECIAL || bits == I_CHAR_SPECIAL) retval =  ENOTDIR;

   put_ext2_inode(pin);

   if (!retval) pin->i_mountpoint = 1;

   return retval;
}

PUBLIC int ext2_sync()
{
	ext2_sync_inodes();
	ext2_sync_buffers();
	return 0;
}
