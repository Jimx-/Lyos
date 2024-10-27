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

#include <lyos/ipc.h>
#include "errno.h"
#include "stdio.h"
#include <stdarg.h>
#include "lyos/const.h"
#include "string.h"

#include "type.h"

#define BUF_SIZE 4096

void init_buf(struct sysfs_buf* buf, char* ptr, size_t len, off_t off)
{
    buf->buf = ptr;
    buf->offset = off;
    buf->left = min(len, BUF_SIZE);
    buf->used = 0;
}

void buf_printf(struct sysfs_buf* buf, char* fmt, ...)
{
    va_list args;
    ssize_t len, max;

    if (buf->left == 0) return;

    max = min(buf->offset + buf->left + 1, BUF_SIZE);

    va_start(args, fmt);
    len = vsnprintf(&buf->buf[buf->used], max, fmt, args);
    va_end(args);

    /*
     * The snprintf family returns the number of bytes that would be stored
     * if the buffer were large enough, excluding the null terminator.
     */
    if (len >= BUF_SIZE) len = BUF_SIZE - 1;

    if (buf->offset > 0) {

        if (buf->offset >= len) {
            buf->offset -= len;

            return;
        }

        memmove(buf, &buf[buf->offset], len - buf->offset);

        len -= buf->offset;
        buf->offset = 0;
    }

    if (len > (ssize_t)buf->left) len = buf->left;

    buf->used += len;
    buf->left -= len;
}

size_t buf_used(struct sysfs_buf* buf) { return buf->used; }
