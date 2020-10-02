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
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <lyos/const.h>
#include <stdint.h>
#include <string.h>
#include <lyos/proc.h>
#include <lyos/driver.h>
#include <lyos/fs.h>
#include <lyos/sysutils.h>
#include <sys/socket.h>
#include <lyos/mgrant.h>
#include <fcntl.h>

#include "const.h"
#include "types.h"
#include "global.h"
#include "proto.h"
#include "poll.h"

struct sdmap {
    char label[FS_LABEL_MAX + 1];
    unsigned int num;
    endpoint_t endpoint;
};

static struct sdmap sdmap[NR_SOCKDEVS];
static struct sdmap* pfmap[PF_MAX];

void init_sdev(void)
{
    int i;
    for (i = 0; i < NR_SOCKDEVS; i++) {
        sdmap[i].num = i + 1;
        sdmap[i].endpoint = NO_TASK;
    }

    memset(pfmap, 0, sizeof(pfmap));
}

int sdev_mapdriver(const char* label, endpoint_t endpoint, const int* domains,
                   int nr_domains)
{
    int i;
    struct sdmap* sdp = NULL;

    for (i = 0; i < NR_SOCKDEVS; i++) {
        if (sdmap[i].endpoint != NO_TASK && !strcmp(sdmap[i].label, label)) {
            sdp = &sdmap[i];
            break;
        }
    }

    for (i = 0; i < nr_domains; i++) {
        if (domains[i] <= 0 || domains[i] >= PF_MAX) return EINVAL;
        if (domains[i] == PF_UNSPEC) return EINVAL;
        if (pfmap[domains[i]] != sdp) return EINVAL;
    }

    if (sdp) {
        for (i = 0; i < PF_MAX; i++) {
            if (pfmap[i] == sdp) pfmap[i] = NULL;
        }
    }

    if (!sdp) {
        for (i = 0; i < NR_SOCKDEVS; i++) {
            if (sdmap[i].endpoint == NO_TASK) {
                sdp = &sdmap[i];
                break;
            }
        }

        if (i == NR_SOCKDEVS) return ENOMEM;
    }

    sdp->endpoint = endpoint;
    strlcpy(sdp->label, label, sizeof(sdp->label));

    for (i = 0; i < nr_domains; i++) {
        pfmap[domains[i]] = sdp;
    }

    return 0;
}

static struct sdmap* get_sdmap_by_domain(int domain)
{
    if (domain <= 0 || domain >= PF_MAX) return NULL;

    return pfmap[domain];
}

static dev_t make_sdmap_dev(struct sdmap* sdp, sockid_t sockid)
{
    return (dev_t)(((uint32_t)sdp->num << 24) | ((uint32_t)sockid & 0xffffff));
}

static struct sdmap* get_sdmap_by_dev(dev_t dev, sockid_t* sockidp)
{
    struct sdmap* sdp;
    int num = (dev >> 24) & 0xff;
    sockid_t id = (sockid_t)(dev & 0xffffff);
    if (!num || num > sizeof(sdmap) / sizeof(sdmap[0]) || id < 0) return NULL;

    sdp = &sdmap[num - 1];
    if (sdp->endpoint == NO_TASK) return NULL;

    if (sockidp) *sockidp = id;
    return sdp;
}

static int sdev_sendrec(struct sdmap* sdp, MESSAGE* msg)
{
    int retval;

    if ((retval = asyncsend3(sdp->endpoint, msg, 0)) != 0) return retval;

    self->recv_from = sdp->endpoint;
    self->msg_driver = msg;

    worker_wait();
    self->recv_from = NO_TASK;

    return 0;
}

static int sdev_simple(int type, endpoint_t src, dev_t dev, int param)
{
    struct sdmap* sdp;
    sockid_t sockid;
    MESSAGE msg;

    if ((sdp = get_sdmap_by_dev(dev, &sockid)) == NULL) return EIO;

    memset(&msg, 0, sizeof(msg));
    msg.type = type;
    msg.u.m_sockdriver_simple.req_id = src;
    msg.u.m_sockdriver_simple.sock_id = sockid;
    msg.u.m_sockdriver_simple.param = param;

    if (sdev_sendrec(sdp, &msg)) {
        panic("vfs: sdev_simple failed to send request");
    }

    return msg.u.m_sockdriver_reply.status;
}

int sdev_socket(endpoint_t src, int domain, int type, int protocol, dev_t* dev,
                int pair)
{
    struct sdmap* sdp;
    MESSAGE msg;
    int sock_id, sock_id2;
    int retval;

    if (!(sdp = get_sdmap_by_domain(domain))) return EAFNOSUPPORT;

    memset(&msg, 0, sizeof(msg));
    msg.type = pair ? SDEV_SOCKETPAIR : SDEV_SOCKET;
    msg.u.m_sockdriver_socket.req_id = src;
    msg.u.m_sockdriver_socket.endpoint = src;
    msg.u.m_sockdriver_socket.domain = domain;
    msg.u.m_sockdriver_socket.type = type;
    msg.u.m_sockdriver_socket.protocol = protocol;

    if (sdev_sendrec(sdp, &msg)) {
        panic("vfs: sdev_socket failed to send request");
    }

    if (msg.type != SDEV_SOCKET_REPLY) return EIO;

    retval = msg.u.m_sockdriver_socket_reply.status;
    if (retval) return retval;

    sock_id = msg.u.m_sockdriver_socket_reply.sock_id;
    sock_id2 = msg.u.m_sockdriver_socket_reply.sock_id2;

    dev[0] = make_sdmap_dev(sdp, sock_id);

    return 0;
}

static int sdev_bindconn(int type, endpoint_t src, dev_t dev, void* addr,
                         size_t addrlen, int flags)
{
    struct sdmap* sdp;
    sockid_t sockid;
    mgrant_id_t grant;
    MESSAGE msg;
    int retval;

    if ((sdp = get_sdmap_by_dev(dev, &sockid)) == NULL) return EIO;

    grant = mgrant_set_proxy(sdp->endpoint, src, (vir_bytes)addr, addrlen,
                             MGF_READ);
    if (grant == GRANT_INVALID)
        panic("vfs: sdev_bind failed to create proxy grant");

    memset(&msg, 0, sizeof(msg));
    msg.type = type;
    msg.u.m_sockdriver_bindconn.req_id = src;
    msg.u.m_sockdriver_bindconn.sock_id = sockid;
    msg.u.m_sockdriver_bindconn.grant = grant;
    msg.u.m_sockdriver_bindconn.len = addrlen;
    msg.u.m_sockdriver_bindconn.user_endpoint = src;
    msg.u.m_sockdriver_bindconn.flags =
        (flags & O_NONBLOCK) ? SDEV_NONBLOCK : 0;

    if (sdev_sendrec(sdp, &msg)) {
        panic("vfs: sdev_socket failed to send request");
    }

    retval = msg.u.m_sockdriver_reply.status;

    mgrant_revoke(grant);

    return retval;
}

int sdev_bind(endpoint_t src, dev_t dev, void* addr, size_t addrlen, int flags)
{
    return sdev_bindconn(SDEV_BIND, src, dev, addr, addrlen, flags);
}

int sdev_connect(endpoint_t src, dev_t dev, void* addr, size_t addrlen,
                 int flags)
{
    return sdev_bindconn(SDEV_CONNECT, src, dev, addr, addrlen, flags);
}

int sdev_listen(endpoint_t src, dev_t dev, int backlog)
{
    return sdev_simple(SDEV_LISTEN, src, dev, backlog);
}

int sdev_accept(endpoint_t src, dev_t dev, void* addr, size_t* addrlen,
                int flags, dev_t* newdev)
{
    struct sdmap* sdp;
    sockid_t sockid;
    mgrant_id_t grant;
    size_t len = *addrlen;
    MESSAGE msg;
    int retval;

    if ((sdp = get_sdmap_by_dev(dev, &sockid)) == NULL) return EIO;

    grant =
        mgrant_set_proxy(sdp->endpoint, src, (vir_bytes)addr, len, MGF_WRITE);
    if (grant == GRANT_INVALID)
        panic("vfs: sdev_bind failed to create proxy grant");

    memset(&msg, 0, sizeof(msg));
    msg.type = SDEV_ACCEPT;
    msg.u.m_sockdriver_bindconn.req_id = src;
    msg.u.m_sockdriver_bindconn.sock_id = sockid;
    msg.u.m_sockdriver_bindconn.grant = grant;
    msg.u.m_sockdriver_bindconn.len = len;
    msg.u.m_sockdriver_bindconn.user_endpoint = src;
    msg.u.m_sockdriver_bindconn.flags =
        (flags & O_NONBLOCK) ? SDEV_NONBLOCK : 0;

    if (sdev_sendrec(sdp, &msg)) {
        panic("vfs: sdev_socket failed to send request");
    }
    retval = msg.u.m_sockdriver_accept_reply.status;

    mgrant_revoke(grant);

    sockid = msg.u.m_sockdriver_accept_reply.sock_id;
    if (sockid >= 0)
        *newdev = make_sdmap_dev(sdp, sockid);
    else
        *newdev = NO_DEV;

    if (retval != OK) return retval;

    *addrlen = msg.u.m_sockdriver_accept_reply.len;

    return 0;
}

ssize_t sdev_readwrite(endpoint_t src, dev_t dev, void* data_buf,
                       size_t data_len, void* ctl_buf, unsigned int ctl_len,
                       void* addr_buf, unsigned int* addr_len, int flags,
                       int rw_flag, int filp_flags, void* msghdr_buf)
{
    struct sdmap* sdp;
    sockid_t sockid;
    mgrant_id_t data_grant, ctl_grant, addr_grant;
    int access;
    MESSAGE msg;

    data_grant = ctl_grant = addr_grant = GRANT_INVALID;
    access = (rw_flag == READ) ? MGF_WRITE : MGF_READ;

    if ((sdp = get_sdmap_by_dev(dev, &sockid)) == NULL) return EIO;

    if (data_buf) {
        data_grant = mgrant_set_proxy(sdp->endpoint, src, (vir_bytes)data_buf,
                                      data_len, access);
        if (data_grant == GRANT_INVALID)
            panic("vfs: sdev_readwrite() failed to create data grant");
    }
    if (ctl_buf) {
        ctl_grant = mgrant_set_proxy(sdp->endpoint, src, (vir_bytes)ctl_buf,
                                     ctl_len, access);
        if (ctl_grant == GRANT_INVALID)
            panic("vfs: sdev_readwrite() failed to create data grant");
    }
    if (addr_buf) {
        addr_grant = mgrant_set_proxy(sdp->endpoint, src, (vir_bytes)addr_buf,
                                      *addr_len, access);
        if (addr_grant == GRANT_INVALID)
            panic("vfs: sdev_readwrite() failed to create data grant");
    }

    msg.type = (rw_flag == READ) ? SDEV_RECV : SDEV_SEND;
    msg.u.m_sockdriver_sendrecv.req_id = src;
    msg.u.m_sockdriver_sendrecv.sock_id = sockid;
    msg.u.m_sockdriver_sendrecv.data_grant = data_grant;
    msg.u.m_sockdriver_sendrecv.data_len = data_len;
    msg.u.m_sockdriver_sendrecv.ctl_grant = ctl_grant;
    msg.u.m_sockdriver_sendrecv.ctl_len = ctl_len;
    msg.u.m_sockdriver_sendrecv.addr_grant = addr_grant;
    msg.u.m_sockdriver_sendrecv.addr_len = addr_len ? *addr_len : 0;
    msg.u.m_sockdriver_sendrecv.user_endpoint = src;

    if (filp_flags & O_NONBLOCK) flags |= MSG_DONTWAIT;
    msg.u.m_sockdriver_sendrecv.flags = flags;

    if (sdev_sendrec(sdp, &msg)) {
        panic("vfs: sdev_readwrite failed to send request");
    }

    if (data_grant != GRANT_INVALID) mgrant_revoke(data_grant);
    if (ctl_grant != GRANT_INVALID) mgrant_revoke(ctl_grant);
    if (addr_grant != GRANT_INVALID) mgrant_revoke(addr_grant);

    return msg.u.m_sockdriver_reply.status;
}

int sdev_close(dev_t dev, int may_block) { return 0; }

void sdev_reply(MESSAGE* msg)
{
    int req_id = NO_TASK;
    struct fproc* fp;

    switch (msg->type) {
    case SDEV_REPLY:
        req_id = msg->u.m_sockdriver_reply.req_id;
        break;
    case SDEV_SOCKET_REPLY:
        req_id = msg->u.m_sockdriver_socket_reply.req_id;
        break;
    case SDEV_ACCEPT_REPLY:
        req_id = msg->u.m_sockdriver_accept_reply.req_id;
        break;
    }

    fp = vfs_endpt_proc(req_id);
    if (fp == NULL) return;

    struct worker_thread* worker = fp->worker;
    if (worker != NULL && worker->msg_driver != NULL) {
        *worker->msg_driver = *msg;
        worker->msg_driver = NULL;
        worker_wake(worker);
    }
}
