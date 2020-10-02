#ifndef _LIBSOCKDRIVER_SOCK_H_
#define _LIBSOCKDRIVER_SOCK_H_

#include <lyos/const.h>

typedef int32_t sockid_t;

/* Socket events. */
#define SEV_BIND    0x01
#define SEV_CONNECT 0x02
#define SEV_ACCEPT  0x04
#define SEV_SEND    0x08
#define SEV_RECV    0x10
#define SEV_CLOSE   0x20

/* Socket flags. */
#define SFL_SHUT_RD 0x01
#define SFL_SHUT_WR 0x02

struct sock {
    sockid_t id;
    unsigned int flags;
    int domain;
    int type;
    unsigned int sock_opt;
    int err;

    int peek_off;
    int rcvlowat;

    struct list_head hash;
    struct list_head wq;
    struct sk_buff_head recvq;

    const struct sockdriver_ops* ops;
};

struct sockdriver_data {
    endpoint_t endpoint;
    mgrant_id_t grant;
    size_t len;
};

#define sock_sockid(sock)            ((sock)->id)
#define sock_type(sock)              ((sock)->type)
#define sock_recvq(sock)             ((sock)->recvq)
#define sock_is_listening(sock)      (!!((sock)->sock_opt & SO_ACCEPTCONN))
#define sock_is_shutdown(sock, mask) ((sock)->flags & (mask))

static inline int sock_error(struct sock* sock)
{
    int retval = sock->err;
    sock->err = 0;
    return retval;
}

static inline void sock_set_error(struct sock* sock, int err)
{
    sock->err = err;
}

static inline int sock_rcvlowat(const struct sock* sock, int waitall, int len)
{
    int v = waitall ? len : min(sock->rcvlowat, len);

    return v ?: 1;
}

static inline int sock_peek_offset(struct sock* sock, int flags)
{
    if (flags & MSG_PEEK) {
        return sock->peek_off;
    }

    return 0;
}

#endif
