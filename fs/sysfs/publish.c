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
#include <sys/stat.h>
#include "libsysfs/libsysfs.h"
#include "libmemfs/libmemfs.h"

PUBLIC int do_publish(MESSAGE * m)
{
    endpoint_t src = m->source;
    int len = m->NAME_LEN;
    int flags = m->FLAGS;

    char name[PATH_MAX];
    if (len > PATH_MAX) return ENAMETOOLONG;

    /* fetch the name */
    data_copy(SELF, name, src, m->PATHNAME, len);
    name[len] = '\0';

    create_node(name, flags);

    return 0;
}