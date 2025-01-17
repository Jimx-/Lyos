#ifndef _UAPI_LYOS_IPC_H_
#define _UAPI_LYOS_IPC_H_

#include <sys/types.h>
#include <lyos/types.h>

/* When modifying the message size, also modify copy_user_message() to copy the
 * new size. */
#define VERIFY_MESS_SIZE(msg_type) \
    typedef int _VERIFY_##msg_type[sizeof(struct msg_type) == 72 ? 1 : -1]

#define BEGIN_MESS_DECL(name) struct name
#define END_MESS_DECL(name)  \
    __attribute__((packed)); \
    VERIFY_MESS_SIZE(name);

/**
 * MESSAGE mechanism is borrowed from MINIX
 */
struct mess1 { /* 16 bytes */
    int m1i1;
    int m1i2;
    int m1i3;
    int m1i4;

    __u8 _pad[56];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess1);

struct mess2 { /* 16 bytes */
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;

    __u8 _pad[72 - 4 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess2);

struct mess3 { /* 40 bytes */
    int m3i1;
    int m3i2;
    int m3i3;
    int m3i4;
    __u64 m3l1;
    __u64 m3l2;
    void* m3p1;
    void* m3p2;

    __u8 _pad[40 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess3);

struct mess4 { /* 36 bytes */
    __u64 m4l1;
    int m4i1, m4i2, m4i3;
    void *m4p1, *m4p2, *m4p3, *m4p4;

    __u8 _pad[52 - 4 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess4);

struct mess5 { /* 40 bytes */
    int m5i1;
    int m5i2;
    int m5i3;
    int m5i4;
    int m5i5;
    int m5i6;
    int m5i7;
    int m5i8;
    int m5i9;
    int m5i10;

    __u8 _pad[32];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess5);

BEGIN_MESS_DECL(mess_safecopy)
{
    __endpoint_t src_dest;
    __mgrant_id_t grant;
    off_t offset;
    void* addr;
    size_t len;

    __u8 _pad[64 - sizeof(off_t) - sizeof(void*) - sizeof(size_t)];
}
END_MESS_DECL(mess_safecopy)

BEGIN_MESS_DECL(mess_stime)
{
    time_t boot_time;

    __u8 _pad[72 - sizeof(time_t)];
}
END_MESS_DECL(mess_stime)

struct mess_mm_mmap {
    __endpoint_t who;
    off_t offset;
    size_t length;

    dev_t dev;
    ino_t ino;
    int fd;

    int flags;
    int prot;
    void* vaddr;
    __u16 clearend;

    __u8 _pad[54 - sizeof(dev_t) - sizeof(ino_t) - sizeof(off_t) -
              sizeof(size_t) - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_mm_mmap);

struct mess_mm_mremap {
    void* old_addr;
    size_t old_size;
    size_t new_size;
    int flags;
    void* new_addr;

    __u8 _pad[68 - 2 * sizeof(void*) - 2 * sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_mm_mremap);

struct mess_mm_mmap_reply {
    int retval;
    void* retaddr;

    __u8 _pad[68 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_mm_mmap_reply);

struct mess_mm_remap {
    __endpoint_t src;
    __endpoint_t dest;
    void* src_addr;
    void* dest_addr;
    size_t size;
    void* ret_addr;

    __u8 _pad[64 - 3 * sizeof(void*) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_mm_remap);

struct mess_pm_signal {
    int signum;
    void* act;
    void* oldact;
    void* sigret;
    int retval;

    __u8 _pad[64 - 3 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_pm_signal);

struct mess_pm_clone {
    int flags;
    void* stack;
    void* parent_tid;
    void* tls;
    void* child_tid;

    __u8 _pad[68 - 4 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_pm_clone);

BEGIN_MESS_DECL(mess_pm_time)
{
    int clock_id;
    time_t sec;
    time_t nsec;

    __u8 _pad[68 - 2 * sizeof(time_t)];
}
END_MESS_DECL(mess_pm_time)

struct mess_vfs_pathat {
    int dirfd;
    void* pathname;
    int name_len;
    int flags;
    mode_t mode;
    void* buf;
    size_t buf_len;

    __u8 _pad[60 - 2 * sizeof(void*) - sizeof(mode_t) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_pathat);

struct mess_vfs_linkat {
    int fd1;
    void* path1;
    int path1_len;
    int fd2;
    void* path2;
    int path2_len;
    int flags;

    __u8 _pad[52 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_linkat);

struct mess_vfs_select {
    __u32 nfds;
    void* readfds;
    void* writefds;
    void* exceptfds;
    void* timeout;

    __u8 _pad[68 - 4 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_select);

struct mess_vfs_link {
    void* old_path;
    void* new_path;
    size_t old_path_len;
    size_t new_path_len;

    __u8 _pad[72 - 2 * sizeof(void*) - 2 * sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_link);

struct mess_vfs_mount {
    void* source;
    void* target;
    void* label;
    void* data;
    unsigned long flags;
    int source_len;
    int target_len;
    int label_len;

    __u8 _pad[60 - 4 * sizeof(void*) - sizeof(unsigned long)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_mount);

struct mess_vfs_fdpair {
    int retval;
    int fd0;
    int fd1;

    __u8 _pad[60];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_fdpair);

struct mess_vfs_poll {
    void* fds;
    unsigned int nfds;
    int timeout_msecs;

    __u8 _pad[64 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_poll);

struct mess_vfs_signalfd {
    int fd;
    unsigned int mask;
    int flags;

    __u8 _pad[60];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_signalfd);

struct mess_vfs_timerfd {
    int clock_id;
    int flags;
    int fd;

    void* new_value;
    void* old_value;

    __u8 _pad[60 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_timerfd);

struct mess_vfs_epoll {
    int flags;
    int epfd;
    int op;
    int fd;
    int max_events;
    int timeout;
    void* events;

    __u8 _pad[48 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_epoll);

struct mess_vfs_mapdriver {
    void* label;
    int label_len;
    int* domains;
    int nr_domains;

    __u8 _pad[64 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_mapdriver);

BEGIN_MESS_DECL(mess_vfs_socket)
{
    int domain;
    int type;
    int protocol;

    __u8 _pad[60];
}
END_MESS_DECL(mess_vfs_socket)

BEGIN_MESS_DECL(mess_vfs_socketpath)
{
    int status;
    __endpoint_t endpoint;
    __mgrant_id_t grant;
    size_t size;
    int request;
    dev_t dev;
    ino_t num;

    __u8 _pad[56 - sizeof(size_t) - sizeof(dev_t) - sizeof(ino_t)];
}
END_MESS_DECL(mess_vfs_socketpath)

BEGIN_MESS_DECL(mess_vfs_bindconn)
{
    int sock_fd;
    void* addr;
    size_t addr_len;
    int flags;

    __u8 _pad[64 - sizeof(void*) - sizeof(size_t)];
}
END_MESS_DECL(mess_vfs_bindconn)

BEGIN_MESS_DECL(mess_vfs_listen)
{
    int sock_fd;
    int backlog;

    __u8 _pad[64];
}
END_MESS_DECL(mess_vfs_listen)

BEGIN_MESS_DECL(mess_vfs_sendrecv)
{
    int status;

    int sock_fd;
    void* buf;
    size_t len;
    int flags;
    void* addr;
    size_t addr_len;

    __u8 _pad[60 - 2 * sizeof(void*) - 2 * sizeof(size_t)];
}
END_MESS_DECL(mess_vfs_sendrecv)

BEGIN_MESS_DECL(mess_vfs_sockopt)
{
    int status;

    int sock_fd;
    int level;
    int name;
    void* buf;
    size_t len;

    __u8 _pad[56 - sizeof(void*) - sizeof(size_t)];
}
END_MESS_DECL(mess_vfs_sockopt)

BEGIN_MESS_DECL(mess_vfs_copyfd)
{
    __endpoint_t endpoint;
    int fd;
    int how;

    __u8 _pad[60];
}
END_MESS_DECL(mess_vfs_copyfd)

BEGIN_MESS_DECL(mess_vfs_utimensat)
{
    int fd;
    void* pathname;
    int name_len;
    int flags;
    time_t atime;
    time_t mtime;
    long ansec;
    long mnsec;

    __u8 _pad[60 - sizeof(void*) - 2 * sizeof(long) - 2 * sizeof(time_t)];
}
END_MESS_DECL(mess_vfs_utimensat)

BEGIN_MESS_DECL(mess_vfs_fchownat)
{
    int fd;
    void* pathname;
    int name_len;
    uid_t owner;
    gid_t group;
    int flags;

    __u8 _pad[60 - sizeof(void*) - sizeof(uid_t) - sizeof(gid_t)];
}
END_MESS_DECL(mess_vfs_fchownat)

BEGIN_MESS_DECL(mess_vfs_truncate)
{
    int fd;
    off_t offset;

    __u8 _pad[68 - sizeof(off_t)];
}
END_MESS_DECL(mess_vfs_truncate)

BEGIN_MESS_DECL(mess_vfs_inotify)
{
    int fd;
    int wd;
    void* pathname;
    int name_len;
    union {
        __u32 flags;
        __u32 mask;
    };

    __u8 _pad[56 - sizeof(void*)];
}
END_MESS_DECL(mess_vfs_inotify)

BEGIN_MESS_DECL(mess_vfs_fs_readsuper)
{
    dev_t dev;
    unsigned int flags;
    __mgrant_id_t grant;
    size_t data_size;

    __u8 _pad[64 - sizeof(dev_t) - sizeof(size_t)];
}
END_MESS_DECL(mess_vfs_fs_readsuper)

BEGIN_MESS_DECL(mess_vfs_fs_lookup)
{
    dev_t dev;
    ino_t start;
    ino_t root;
    __endpoint_t user_endpt;
    __mgrant_id_t path_grant;
    size_t name_len;
    size_t path_size;

    union {
        struct {
            uid_t uid;
            gid_t gid;
        };

        struct {
            __mgrant_id_t ucred_grant;
            int ucred_size;
        };
    };

    int flags;

    __u8 _pad[52 - sizeof(dev_t) - 2 * sizeof(ino_t) - 2 * sizeof(size_t)];
}
END_MESS_DECL(mess_vfs_fs_lookup)

BEGIN_MESS_DECL(mess_fs_vfs_lookup_reply)
{
    int status;
    ino_t num;

    union {
        struct {
            uid_t uid;
            gid_t gid;
            size_t size;
            mode_t mode;
            dev_t spec_dev;
        } __attribute__((packed)) node;

        off_t offset;
    };

    __u8 _pad[68 - sizeof(dev_t) - sizeof(ino_t) - sizeof(size_t) -
              sizeof(uid_t) - sizeof(gid_t) - sizeof(mode_t)];
}
END_MESS_DECL(mess_fs_vfs_lookup_reply)

BEGIN_MESS_DECL(mess_vfs_fs_stat)
{
    dev_t dev;
    ino_t num;
    __mgrant_id_t grant;

    __u8 _pad[68 - sizeof(dev_t) - sizeof(ino_t)];
}
END_MESS_DECL(mess_vfs_fs_stat)

struct mess_vfs_fs_symlink {
    dev_t dev;
    ino_t dir_ino;
    __mgrant_id_t name_grant;
    size_t name_len;
    __mgrant_id_t target_grant;
    size_t target_len;
    uid_t uid;
    gid_t gid;

    __u8 _pad[64 - sizeof(dev_t) - sizeof(ino_t) - 2 * sizeof(size_t) -
              sizeof(uid_t) - sizeof(gid_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_fs_symlink);

struct mess_vfs_fs_link {
    dev_t dev;
    ino_t dir_ino;
    ino_t inode;
    __mgrant_id_t name_grant;
    size_t name_len;

    __u8 _pad[68 - sizeof(dev_t) - 2 * sizeof(ino_t) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_fs_link);

BEGIN_MESS_DECL(mess_vfs_fs_create)
{
    dev_t dev;
    ino_t num;
    uid_t uid;
    gid_t gid;
    __mgrant_id_t grant;
    int name_len;
    mode_t mode;

    __u8 _pad[64 - sizeof(dev_t) - sizeof(ino_t) - sizeof(uid_t) -
              sizeof(gid_t) - sizeof(mode_t)];
}
END_MESS_DECL(mess_vfs_fs_create)

BEGIN_MESS_DECL(mess_fs_vfs_create_reply)
{
    int status;
    ino_t num;
    uid_t uid;
    gid_t gid;
    mode_t mode;
    size_t size;

    __u8 _pad[68 - sizeof(ino_t) - sizeof(uid_t) - sizeof(gid_t) -
              sizeof(size_t) - sizeof(mode_t)];
}
END_MESS_DECL(mess_fs_vfs_create_reply)

BEGIN_MESS_DECL(mess_vfs_fs_mknod)
{
    dev_t dev;
    ino_t num;
    uid_t uid;
    gid_t gid;
    __mgrant_id_t grant;
    int name_len;
    mode_t mode;
    dev_t sdev;

    __u8 _pad[64 - 2 * sizeof(dev_t) - sizeof(ino_t) - sizeof(uid_t) -
              sizeof(gid_t) - sizeof(mode_t)];
}
END_MESS_DECL(mess_vfs_fs_mknod)

BEGIN_MESS_DECL(mess_vfs_fs_readwrite)
{
    ssize_t status;
    dev_t dev;
    ino_t num;
    __u64 position;
    int rw_flag;
    __mgrant_id_t grant;
    size_t count;
    __endpoint_t user_endpt; /* for rdlink only */

    __u8 _pad[52 - sizeof(dev_t) - sizeof(ino_t) - 2 * sizeof(size_t)];
}
END_MESS_DECL(mess_vfs_fs_readwrite)

BEGIN_MESS_DECL(mess_vfs_fs_unlink)
{
    dev_t dev;
    ino_t num;
    __mgrant_id_t grant;
    size_t name_len;

    __u8 _pad[68 - sizeof(dev_t) - sizeof(ino_t) - sizeof(size_t)];
}
END_MESS_DECL(mess_vfs_fs_unlink)

BEGIN_MESS_DECL(mess_vfs_fs_utime)
{
    dev_t dev;
    ino_t num;
    time_t atime;
    time_t mtime;
    long ansec;
    long mnsec;

    __u8 _pad[72 - sizeof(dev_t) - sizeof(ino_t) - 2 * sizeof(time_t) -
              2 * sizeof(long)];
}
END_MESS_DECL(mess_vfs_fs_utime)

BEGIN_MESS_DECL(mess_vfs_fs_chown)
{
    int status;
    dev_t dev;
    ino_t num;
    uid_t uid;
    gid_t gid;
    mode_t mode;

    __u8 _pad[68 - sizeof(dev_t) - sizeof(ino_t) - sizeof(uid_t) -
              sizeof(gid_t) - sizeof(mode_t)];
}
END_MESS_DECL(mess_vfs_fs_chown)

BEGIN_MESS_DECL(mess_vfs_fs_putinode)
{
    dev_t dev;
    ino_t num;
    unsigned int count;

    __u8 _pad[68 - sizeof(dev_t) - sizeof(ino_t)];
}
END_MESS_DECL(mess_vfs_fs_putinode)

struct mess_vfs_cdev_openclose {
    __u64 minor;
    __u32 id;
    __endpoint_t user;
    int access;

    __u8 _pad[52];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_openclose);

struct mess_vfs_cdev_readwrite {
    __u64 minor;
    __endpoint_t endpoint;
    __u32 id;
    void* buf;
    __mgrant_id_t grant;
    __u32 request;
    off_t pos;
    size_t count;
    int flags;

    __u8 _pad[44 - sizeof(void*) - sizeof(off_t) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_readwrite);

struct mess_vfs_cdev_mmap {
    __u64 minor;
    __u32 id;
    void* addr;
    __endpoint_t endpoint;
    off_t pos;
    size_t count;

    __u8 _pad[56 - sizeof(void*) - sizeof(off_t) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_mmap);

struct mess_vfs_cdev_select {
    __u64 minor;
    __u32 ops;
    __u32 id;

    __u8 _pad[56];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_select);

struct mess_vfs_cdev_reply {
    __s32 status;
    __u32 id;
    void* retaddr;

    __u8 _pad[64 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_reply);

struct mess_vfs_cdev_poll_notify {
    __u64 minor;
    __u32 status;

    __u8 _pad[60];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_poll_notify);

struct mess_vfs_pm_signalfd {
    __endpoint_t endpoint;
    unsigned int sigmask;
    void* buf;
    int notify;

    __u8 _pad[60 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_pm_signalfd);

struct mess_pm_vfs_signalfd_reply {
    int status;
    __endpoint_t endpoint;

    __u8 _pad[64];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_pm_vfs_signalfd_reply);

union sysfs_val {
    __mgrant_id_t grant;
    __u32 u32;
    __endpoint_t endpoint;
    unsigned int attr_id;
};

struct mess_sysfs_req {
    int status;
    __mgrant_id_t key_grant;
    size_t key_len;
    __mgrant_id_t target_grant;
    size_t target_len;
    union sysfs_val val;
    size_t val_len;
    int flags;
    __endpoint_t owner;
    int event;

    __u8 _pad[44 - 3 * sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_sysfs_req);

struct mess_bdev_blockdriver_msg {
    int minor;
    __u64 pos;
    size_t count;
    __mgrant_id_t grant;
    __endpoint_t user_endpoint;
    int request;

    __u8 _pad[48 - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_bdev_blockdriver_msg);

struct mess_blockdriver_bdev_reply {
    ssize_t status;

    __u8 _pad[72 - sizeof(ssize_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_blockdriver_bdev_reply);

struct mess_inputdriver_input_event {
    int id;
    __u16 type;
    __u16 code;
    __s32 value;

    __u8 _pad[60];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_inputdriver_input_event);

struct mess_inputdriver_register_device {
    int dev_id;
    void* dev_bits;
    void* input_id;

    __u8 _pad[68 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_inputdriver_register_device);

struct mess_input_tty_event {
    int id;
    __u16 type;
    __u16 code;
    __s32 value;

    __u8 _pad[60];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_input_tty_event);

struct mess_input_conf {
    int status;
    int id;

    __u8 _pad[64];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_input_conf);

struct mess_devman_register_reply {
    int status;
    int id;

    __u8 _pad[64];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_devman_register_reply);

BEGIN_MESS_DECL(mess_devman_attr_req)
{
    int target;
    void* buf;
    size_t len;
    void* opaque;

    __u8 _pad[68 - 2 * sizeof(void*) - sizeof(size_t)];
}
END_MESS_DECL(mess_devman_attr_req)

BEGIN_MESS_DECL(mess_devman_attr_reply)
{
    int status;
    size_t count;
    void* opaque;

    __u8 _pad[68 - sizeof(void*) - sizeof(size_t)];
}
END_MESS_DECL(mess_devman_attr_reply)

BEGIN_MESS_DECL(mess_sockdriver_simple)
{
    int req_id;
    int sock_id;
    int param;

    __u8 _pad[60];
}
END_MESS_DECL(mess_sockdriver_simple)

BEGIN_MESS_DECL(mess_sockdriver_reply)
{
    int req_id;
    int status;

    __u8 _pad[64];
}
END_MESS_DECL(mess_sockdriver_reply)

BEGIN_MESS_DECL(mess_sockdriver_socket)
{
    int req_id;
    __endpoint_t endpoint;
    int domain;
    int type;
    int protocol;

    __u8 _pad[52];
}
END_MESS_DECL(mess_sockdriver_socket)

BEGIN_MESS_DECL(mess_sockdriver_socket_reply)
{
    int req_id;
    int status;
    int sock_id;
    int sock_id2;

    __u8 _pad[56];
}
END_MESS_DECL(mess_sockdriver_socket_reply)

BEGIN_MESS_DECL(mess_sockdriver_bindconn)
{
    int req_id;
    int sock_id;
    __mgrant_id_t grant;
    size_t len;
    __endpoint_t user_endpoint;
    int flags;

    __u8 _pad[52 - sizeof(size_t)];
}
END_MESS_DECL(mess_sockdriver_bindconn)

BEGIN_MESS_DECL(mess_sockdriver_accept_reply)
{
    int req_id;
    int status;
    int sock_id;
    size_t len;

    __u8 _pad[60 - sizeof(size_t)];
}
END_MESS_DECL(mess_sockdriver_accept_reply)

BEGIN_MESS_DECL(mess_sockdriver_sendrecv)
{
    int req_id;
    int sock_id;
    __mgrant_id_t data_grant;
    size_t data_len;
    __mgrant_id_t ctl_grant;
    unsigned int ctl_len;
    __mgrant_id_t addr_grant;
    unsigned int addr_len;
    __endpoint_t user_endpoint;
    int flags;

    __u8 _pad[36 - sizeof(size_t)];
}
END_MESS_DECL(mess_sockdriver_sendrecv)

BEGIN_MESS_DECL(mess_sockdriver_recv_reply)
{
    int req_id;
    int status;
    size_t ctl_len;
    size_t addr_len;

    __u8 _pad[64 - 2 * sizeof(size_t)];
}
END_MESS_DECL(mess_sockdriver_recv_reply)

BEGIN_MESS_DECL(mess_sockdriver_select)
{
    int status;
    int req_id;
    int sock_id;
    __u32 ops;

    __u8 _pad[56];
}
END_MESS_DECL(mess_sockdriver_select)

BEGIN_MESS_DECL(mess_sockdriver_poll_notify)
{
    int sock_id;
    __poll_t ops;

    __u8 _pad[64];
}
END_MESS_DECL(mess_sockdriver_poll_notify)

BEGIN_MESS_DECL(mess_sockdriver_getset)
{
    int req_id;
    int sock_id;
    int level;
    int name;
    __mgrant_id_t grant;
    size_t len;

    __u8 _pad[52 - sizeof(size_t)];
}
END_MESS_DECL(mess_sockdriver_getset)

BEGIN_MESS_DECL(mess_sockdriver_ioctl)
{
    int req_id;
    int sock_id;
    unsigned long request;
    __mgrant_id_t grant;
    __endpoint_t endpoint;
    int flags;

    __u8 _pad[52 - sizeof(unsigned long)];
}
END_MESS_DECL(mess_sockdriver_ioctl)

BEGIN_MESS_DECL(mess_devpts_req)
{
    int status;
    unsigned int index;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    dev_t dev;

    __u8 _pad[64 - sizeof(mode_t) - sizeof(uid_t) - sizeof(gid_t) -
              sizeof(dev_t)];
}
END_MESS_DECL(mess_devpts_req)

BEGIN_MESS_DECL(mess_ndev_init)
{
    unsigned int id;
    int status;

    __u8 _pad[64];
}
END_MESS_DECL(mess_ndev_init)

BEGIN_MESS_DECL(mess_ndev_init_reply)
{
    unsigned int id;
    __u8 hwaddr[6];
    __u8 hwaddr_len;
    __u8 max_send;
    __u8 max_recv;
    __u8 link;

    __u8 _pad1[58];
}
END_MESS_DECL(mess_ndev_init_reply)

#define NDEV_TRANSFER_INLINE_IOVS 8
BEGIN_MESS_DECL(mess_ndev_transfer)
{
    unsigned int id;
    unsigned int count;
    __mgrant_id_t grant[NDEV_TRANSFER_INLINE_IOVS];
    unsigned int len[NDEV_TRANSFER_INLINE_IOVS];

    __u8 _pad[64 - 8 * NDEV_TRANSFER_INLINE_IOVS];
}
END_MESS_DECL(mess_ndev_transfer)

BEGIN_MESS_DECL(mess_ndev_reply)
{
    unsigned int id;
    int status;

    __u8 _pad[64];
}
END_MESS_DECL(mess_ndev_reply)

typedef struct {
    int source;
    int type;
    union {
        struct mess1 m1;
        struct mess2 m2;
        struct mess3 m3;
        struct mess4 m4;
        struct mess5 m5;
        struct mess_safecopy m_safecopy;
        struct mess_stime m_stime;
        struct mess_mm_mmap m_mm_mmap;
        struct mess_mm_mremap m_mm_mremap;
        struct mess_mm_mmap_reply m_mm_mmap_reply;
        struct mess_mm_remap m_mm_remap;
        struct mess_pm_signal m_pm_signal;
        struct mess_pm_clone m_pm_clone;
        struct mess_pm_time m_pm_time;
        struct mess_vfs_pathat m_vfs_pathat;
        struct mess_vfs_linkat m_vfs_linkat;
        struct mess_vfs_select m_vfs_select;
        struct mess_vfs_link m_vfs_link;
        struct mess_vfs_mount m_vfs_mount;
        struct mess_vfs_fdpair m_vfs_fdpair;
        struct mess_vfs_poll m_vfs_poll;
        struct mess_vfs_signalfd m_vfs_signalfd;
        struct mess_vfs_timerfd m_vfs_timerfd;
        struct mess_vfs_epoll m_vfs_epoll;
        struct mess_vfs_mapdriver m_vfs_mapdriver;
        struct mess_vfs_socket m_vfs_socket;
        struct mess_vfs_socketpath m_vfs_socketpath;
        struct mess_vfs_bindconn m_vfs_bindconn;
        struct mess_vfs_listen m_vfs_listen;
        struct mess_vfs_sendrecv m_vfs_sendrecv;
        struct mess_vfs_sockopt m_vfs_sockopt;
        struct mess_vfs_copyfd m_vfs_copyfd;
        struct mess_vfs_utimensat m_vfs_utimensat;
        struct mess_vfs_fchownat m_vfs_fchownat;
        struct mess_vfs_truncate m_vfs_truncate;
        struct mess_vfs_inotify m_vfs_inotify;
        struct mess_vfs_fs_readsuper m_vfs_fs_readsuper;
        struct mess_vfs_fs_lookup m_vfs_fs_lookup;
        struct mess_fs_vfs_lookup_reply m_fs_vfs_lookup_reply;
        struct mess_vfs_fs_stat m_vfs_fs_stat;
        struct mess_vfs_fs_symlink m_vfs_fs_symlink;
        struct mess_vfs_fs_link m_vfs_fs_link;
        struct mess_vfs_fs_create m_vfs_fs_create;
        struct mess_vfs_fs_mknod m_vfs_fs_mknod;
        struct mess_fs_vfs_create_reply m_fs_vfs_create_reply;
        struct mess_vfs_fs_readwrite m_vfs_fs_readwrite;
        struct mess_vfs_fs_unlink m_vfs_fs_unlink;
        struct mess_vfs_fs_utime m_vfs_fs_utime;
        struct mess_vfs_fs_chown m_vfs_fs_chown;
        struct mess_vfs_fs_putinode m_vfs_fs_putinode;
        struct mess_vfs_cdev_openclose m_vfs_cdev_openclose;
        struct mess_vfs_cdev_readwrite m_vfs_cdev_readwrite;
        struct mess_vfs_cdev_mmap m_vfs_cdev_mmap;
        struct mess_vfs_cdev_select m_vfs_cdev_select;
        struct mess_vfs_cdev_reply m_vfs_cdev_reply;
        struct mess_vfs_cdev_poll_notify m_vfs_cdev_poll_notify;
        struct mess_vfs_pm_signalfd m_vfs_pm_signalfd;
        struct mess_pm_vfs_signalfd_reply m_pm_vfs_signalfd_reply;
        struct mess_sysfs_req m_sysfs_req;
        struct mess_bdev_blockdriver_msg m_bdev_blockdriver_msg;
        struct mess_blockdriver_bdev_reply m_blockdriver_bdev_reply;
        struct mess_inputdriver_register_device m_inputdriver_register_device;
        struct mess_inputdriver_input_event m_inputdriver_input_event;
        struct mess_input_tty_event m_input_tty_event;
        struct mess_input_conf m_input_conf;
        struct mess_devman_register_reply m_devman_register_reply;
        struct mess_devman_attr_req m_devman_attr_req;
        struct mess_devman_attr_reply m_devman_attr_reply;
        struct mess_sockdriver_simple m_sockdriver_simple;
        struct mess_sockdriver_reply m_sockdriver_reply;
        struct mess_sockdriver_socket m_sockdriver_socket;
        struct mess_sockdriver_socket_reply m_sockdriver_socket_reply;
        struct mess_sockdriver_bindconn m_sockdriver_bindconn;
        struct mess_sockdriver_accept_reply m_sockdriver_accept_reply;
        struct mess_sockdriver_sendrecv m_sockdriver_sendrecv;
        struct mess_sockdriver_recv_reply m_sockdriver_recv_reply;
        struct mess_sockdriver_poll_notify m_sockdriver_poll_notify;
        struct mess_sockdriver_select m_sockdriver_select;
        struct mess_sockdriver_getset m_sockdriver_getset;
        struct mess_sockdriver_ioctl m_sockdriver_ioctl;
        struct mess_devpts_req m_devpts_req;
        struct mess_ndev_init m_ndev_init;
        struct mess_ndev_init_reply m_ndev_init_reply;
        struct mess_ndev_transfer m_ndev_transfer;
        struct mess_ndev_reply m_ndev_reply;

        __u8 m_payload[56];
    } u;
} __attribute__((packed)) MESSAGE;
typedef int _VERIFY_MESSAGE[sizeof(MESSAGE) == 80 ? 1 : -1];

int send_recv(int function, int src_dest, MESSAGE* msg);

#undef VERIFY_MESS_SIZE
#undef BEGIN_MESS_DECL
#undef END_MESS_DECL

#endif
