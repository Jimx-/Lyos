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

#ifndef _SYSFS_PROTO_H_
#define _SYSFS_PROTO_H_

#include "type.h"

void init_node();
sysfs_node_t* new_node(const char* name, int flags);
int add_node(sysfs_node_t* parent, sysfs_node_t* child);
sysfs_node_t* find_node(sysfs_node_t* parent, const char* name);
sysfs_node_t* lookup_node_by_name(const char* name);
sysfs_node_t* create_node(const char* name, int flags);
int traverse_node(sysfs_node_t* root, int type_mask,
                  int (*callback)(const char*, sysfs_node_t*, void*),
                  void* cb_data);

void init_buf(struct sysfs_buf* buf, char* ptr, size_t len, off_t off);
void buf_printf(struct sysfs_buf* buf, char* fmt, ...);
size_t buf_used(struct sysfs_buf* buf);

ssize_t sysfs_read_hook(struct memfs_inode* inode, char* ptr, size_t count,
                        off_t offset, cbdata_t data);
ssize_t sysfs_write_hook(struct memfs_inode* inode, char* ptr, size_t count,
                         off_t offset, cbdata_t data);
int sysfs_rdlink_hook(struct memfs_inode* inode, char* ptr, size_t max,
                      endpoint_t user_endpt, cbdata_t data);

int do_publish(MESSAGE* m);
int do_publish_link(MESSAGE* m);
int do_retrieve(MESSAGE* m);
int do_subscribe(MESSAGE* m);
int do_get_event(MESSAGE* m);

void do_dyn_attr_reply(const MESSAGE* msg);

#endif
