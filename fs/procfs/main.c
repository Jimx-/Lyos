/*
    (c)Copyright 2011 Jimx

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

#include <lyos/ipc.h>
#include "lyos/const.h"
#include "string.h"
#include <lyos/sysutils.h>
#include <lyos/service.h>
#include <sys/stat.h>
#include "libmemfs/libmemfs.h"
#include "global.h"
#include "proto.h"

#define MAX_THREADS 8

static int init_procfs();
static void procfs_init_hook();

struct memfs_hooks fs_hooks = {
    .init_hook = procfs_init_hook,
    .read_hook = procfs_read_hook,
    .getdents_hook = procfs_getdents_hook,
    .lookup_hook = procfs_lookup_hook,
    .rdlink_hook = procfs_rdlink_hook,
};

int main()
{
    serv_register_init_fresh_callback(init_procfs);
    serv_init();

    struct memfs_stat root_stat;
    root_stat.st_mode =
        (S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    root_stat.st_uid = SU_UID;
    root_stat.st_gid = 0;

    return memfs_start("proc", &fs_hooks, &root_stat, MAX_THREADS);
}

static int init_procfs()
{
    printl("procfs: procfs is running.\n");

    return 0;
}

static void build_root(struct memfs_inode* root)
{
    struct memfs_stat stat;

    memset(&stat, 0, sizeof(stat));
    stat.st_uid = SU_UID;
    stat.st_gid = 0;
    stat.st_size = 0;

    struct procfs_file* fp;
    for (fp = root_files; fp->name != NULL; fp++) {
        stat.st_mode = fp->mode;
        struct memfs_inode* pin =
            memfs_add_inode(root, fp->name, NO_INDEX, &stat, fp->data);
        if (pin == NULL) return;
    }
}

static void procfs_init_hook()
{
    static int first = 1;

    if (first) {
        struct memfs_inode* root = memfs_get_root_inode();

        build_root(root);

        first = 0;
    }
}
