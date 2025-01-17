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
#include <stdlib.h>
#include "errno.h"
#include "lyos/const.h"
#include <lyos/sysutils.h>
#include <lyos/mgrant.h>
#include "string.h"
#include "libsysfs.h"

#define DYN_ATTR_HASH_LOG2 7
#define DYN_ATTR_HASH_SIZE ((unsigned long)1 << DYN_ATTR_HASH_LOG2)
#define DYN_ATTR_HASH_MASK (((unsigned long)1 << DYN_ATTR_HASH_LOG2) - 1)

/* dynamic attribute hash table */
static struct list_head dyn_attr_table[DYN_ATTR_HASH_SIZE];

int sysfs_init_dyn_attr(sysfs_dyn_attr_t* attr, char* label, int flags,
                        void* cb_data, sysfs_dyn_attr_show_t show,
                        sysfs_dyn_attr_store_t store)
{
    static int initialized = 0;
    if (!initialized) {
        int i;
        for (i = 0; i < DYN_ATTR_HASH_SIZE; i++) {
            INIT_LIST_HEAD(&dyn_attr_table[i]);
        }
        initialized = 1;
    }

    if (strlen(label) >= NAME_MAX) {
        return ENAMETOOLONG;
    }
    strlcpy(attr->label, label, NAME_MAX);
    attr->label[NAME_MAX - 1] = '\0';

    attr->flags = flags;
    attr->cb_data = cb_data;
    attr->show = show;
    attr->store = store;
    INIT_LIST_HEAD(&attr->list);

    return 0;
}

static void dyn_attr_addhash(sysfs_dyn_attr_t* attr)
{
    /* Add a dynamic attribute to hash table */
    unsigned int hash = attr->attr_id & DYN_ATTR_HASH_MASK;
    list_add(&attr->list, &dyn_attr_table[hash]);
}

/*
static void dyn_attr_unhash(sysfs_dyn_attr_t* attr)
{
    list_del(&attr->list);
}
*/

static sysfs_dyn_attr_t* find_dyn_attr_by_id(sysfs_dyn_attr_id_t id)
{
    /* Find a dynamic attribute by its attribute id */
    unsigned int hash = id & DYN_ATTR_HASH_MASK;

    sysfs_dyn_attr_t* attr;
    list_for_each_entry(attr, &dyn_attr_table[hash], list)
    {
        if (attr->attr_id == id) {
            return attr;
        }
    }

    return NULL;
}

int sysfs_publish_dyn_attr(sysfs_dyn_attr_t* attr)
{
    MESSAGE msg;
    mgrant_id_t grant;
    size_t key_len;

    key_len = strlen(attr->label) + 1;

    grant = mgrant_set_direct(TASK_SYSFS, (vir_bytes)attr->label, key_len,
                              MGF_READ);
    if (grant == GRANT_INVALID) return ENOMEM;

    memset(&msg, 0, sizeof(msg));
    msg.type = SYSFS_PUBLISH;
    msg.u.m_sysfs_req.key_grant = grant;
    msg.u.m_sysfs_req.key_len = key_len;
    msg.u.m_sysfs_req.flags = attr->flags | SF_TYPE_DYNAMIC;

    send_recv(BOTH, TASK_SYSFS, &msg);

    mgrant_revoke(grant);

    if (msg.u.m_sysfs_req.status) return msg.u.m_sysfs_req.status;

    sysfs_dyn_attr_t* new_attr =
        (sysfs_dyn_attr_t*)malloc(sizeof(sysfs_dyn_attr_t));
    if (!new_attr) return ENOMEM;
    memcpy(new_attr, attr, sizeof(sysfs_dyn_attr_t));

    sysfs_dyn_attr_id_t id = (sysfs_dyn_attr_id_t)msg.u.m_sysfs_req.val.attr_id;
    new_attr->attr_id = id;

    dyn_attr_addhash(new_attr);

    return 0;
}

#define BUFSIZE 4096
void sysfs_handle_dyn_attr(MESSAGE* msg)
{
    /* handle dynamic attribute show/store request */
    int rw_flag = msg->type;
    static char tmp_buf[BUFSIZE];
    size_t count = msg->CNT;
    char* buf = msg->BUF;
    ssize_t retval;

    sysfs_dyn_attr_t* attr = find_dyn_attr_by_id(msg->TARGET);
    if (!attr) {
        retval = -ENOENT;
        goto reply;
    }

    memset(tmp_buf, 0, sizeof(tmp_buf));

    if (rw_flag == SYSFS_DYN_SHOW) {
        if (!attr->show) {
            retval = -EPERM;
            goto reply;
        }

        retval = attr->show(attr, tmp_buf, sizeof(tmp_buf));
        if (retval >= count) {
            retval = -E2BIG;
            goto reply;
        }
        if (retval < 0) goto reply;

        data_copy(msg->source, buf, SELF, tmp_buf, retval);
    } else {
        if (!attr->store) {
            retval = -EPERM;
            goto reply;
        }

        if (count >= BUFSIZE) {
            retval = -E2BIG;
            goto reply;
        }

        data_copy(SELF, tmp_buf, msg->source, buf, count);

        retval = attr->store(attr, tmp_buf, count);
    }

reply:
    if (retval == SUSPEND) return;

    msg->CNT = retval;

    msg->type = SYSFS_DYN_REPLY;
    send_recv(SEND, msg->source, msg);
}
