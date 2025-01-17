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

#ifndef _LIBMEMFS_H_
#define _LIBMEMFS_H_

#include <lyos/list.h>
#include <libfsdriver/libfsdriver.h>

#include "inode.h"

#define NO_INDEX -1

struct memfs_hooks {
    void (*init_hook)(void);
    ssize_t (*read_hook)(struct memfs_inode* inode, char* ptr, size_t count,
                         off_t offset, cbdata_t data);
    ssize_t (*write_hook)(struct memfs_inode* inode, char* ptr, size_t count,
                          off_t offset, cbdata_t data);
    int (*getdents_hook)(struct memfs_inode* inode, cbdata_t data);
    int (*lookup_hook)(struct memfs_inode* parent, const char* name,
                       cbdata_t data);
    int (*symlink_hook)(struct memfs_inode* parent, const char* name,
                        struct memfs_stat* stat, const char* path,
                        cbdata_t cbdata);
    int (*rdlink_hook)(struct memfs_inode* inode, char* ptr, size_t max,
                       endpoint_t user_endpt, cbdata_t data);
    int (*mkdir_hook)(struct memfs_inode* parent, const char* name,
                      struct memfs_stat* stat, cbdata_t cbdata);
    int (*mknod_hook)(struct memfs_inode* parent, const char* name,
                      struct memfs_stat* stat, cbdata_t cbdata);
    int (*chstat_hook)(struct memfs_inode* inode, struct memfs_stat* stat,
                       cbdata_t cbdata);
    void (*message_hook)(MESSAGE* m);
};

extern struct memfs_hooks fs_hooks;

int memfs_start(char* name, struct memfs_hooks* hooks,
                struct memfs_stat* root_stat, size_t num_workers);

int memfs_node_index(struct memfs_inode* pin);
struct memfs_inode* memfs_node_parent(struct memfs_inode* pin);
struct memfs_inode* memfs_add_inode(struct memfs_inode* parent,
                                    const char* name, int index,
                                    struct memfs_stat* stat, cbdata_t data);
struct memfs_inode* memfs_get_root_inode();
struct memfs_inode* memfs_find_inode_by_name(struct memfs_inode* parent,
                                             const char* name);
struct memfs_inode* memfs_find_inode_by_index(struct memfs_inode* parent,
                                              int index);

void memfs_get_inode_stat(struct memfs_inode* pin, struct memfs_stat* stat);
void memfs_set_inode_stat(struct memfs_inode* pin, struct memfs_stat* stat);

#endif
