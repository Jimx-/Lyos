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

#include <lyos/types.h>
#include <lyos/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <lyos/const.h>
#include "libmemfs/libmemfs.h"
#include "proto.h"

#define BUFSIZE 1024

static ssize_t memfs_readwrite(dev_t dev, ino_t num, int rw_flag,
                               struct fsdriver_data* data, loff_t rwpos,
                               size_t count)
{
    struct memfs_inode* pin = memfs_find_inode(num);
    off_t off;
    size_t chunk;
    ssize_t len;
    char* buf;
    size_t buf_size;
    int retval = 0;

    if (!pin) return -ENOENT;

    if (!S_ISREG(pin->i_stat.st_mode)) return -EINVAL;

    if (rw_flag == READ && fs_hooks.read_hook == NULL) {
        return 0;
    }
    if (rw_flag == WRITE && fs_hooks.write_hook == NULL) {
        return 0;
    }

    buf = malloc(BUFSIZE);
    if (!buf) return -ENOMEM;
    buf_size = BUFSIZE;

    for (off = 0; off < count;) {
        chunk = count - off;
        if (chunk > buf_size) chunk = buf_size;

        if (rw_flag == WRITE) {
            retval = fsdriver_copyin(data, off, buf, chunk);
        }

        if (rw_flag == READ) {
            len = fs_hooks.read_hook(pin, buf, chunk, rwpos, pin->data);
        } else {
            len = fs_hooks.write_hook(pin, buf, chunk, rwpos, pin->data);
        }

        if (len > 0) {
            if (rw_flag == READ) retval = fsdriver_copyout(data, off, buf, len);
        } else {
            retval = -len;
        }

        if (retval) {
            off = off > 0 ? off : -retval;
            goto free_buf;
        }

        off += len;
        rwpos += len;

        if (len < buf_size) break;
    }

free_buf:
    free(buf);

    return off;
}

ssize_t memfs_read(dev_t dev, ino_t num, struct fsdriver_data* data,
                   loff_t rwpos, size_t count)
{
    return memfs_readwrite(dev, num, READ, data, rwpos, count);
}

ssize_t memfs_write(dev_t dev, ino_t num, struct fsdriver_data* data,
                    loff_t rwpos, size_t count)
{
    return memfs_readwrite(dev, num, WRITE, data, rwpos, count);
}
