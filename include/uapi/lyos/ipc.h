#ifndef _UAPI_LYOS_IPC_H_
#define _UAPI_LYOS_IPC_H_

#include <sys/types.h>
#include <lyos/types.h>

#define VERIFY_MESS_SIZE(msg_type) \
    typedef int _VERIFY_##msg_type[sizeof(struct msg_type) == 56 ? 1 : -1]

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

    __u8 _pad[40];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess1);

struct mess2 { /* 16 bytes */
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;

    __u8 _pad[56 - 4 * sizeof(void*)];
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

    __u8 _pad[24 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess3);

struct mess4 { /* 36 bytes */
    __u64 m4l1;
    int m4i1, m4i2, m4i3;
    void *m4p1, *m4p2, *m4p3, *m4p4;

    __u8 _pad[36 - 4 * sizeof(void*)];
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

    __u8 _pad[16];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess5);

BEGIN_MESS_DECL(mess_safecopy)
{
    __endpoint_t src_dest;
    __mgrant_id_t grant;
    __u64 offset;
    void* addr;
    size_t len;

    __u8 _pad[40 - sizeof(void*) - sizeof(size_t)];
}
END_MESS_DECL(mess_safecopy)

struct mess_mm_mmap {
    __endpoint_t who;
    size_t offset;
    size_t length;

    union {
        struct {
            __u64 dev;
            __u32 ino;
        } __attribute__((packed)) devino;
        int fd;
    };

    int flags;
    int prot;
    void* vaddr;
    size_t clearend;

    __u8 _pad[32 - 3 * sizeof(size_t) - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_mm_mmap);

struct mess_mm_mmap_reply {
    int retval;
    void* retaddr;

    __u8 _pad[52 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_mm_mmap_reply);

struct mess_mm_remap {
    __endpoint_t src;
    __endpoint_t dest;
    void* src_addr;
    void* dest_addr;
    size_t size;
    void* ret_addr;

    __u8 _pad[48 - 3 * sizeof(void*) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_mm_remap);

struct mess_pm_signal {
    int signum;
    void* act;
    void* oldact;
    void* sigret;
    int retval;

    __u8 _pad[48 - 3 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_pm_signal);

struct mess_pm_clone {
    int flags;
    void* stack;
    void* parent_tid;
    void* tls;
    void* child_tid;

    __u8 _pad[52 - 4 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_pm_clone);

struct mess_vfs_select {
    __u32 nfds;
    void* readfds;
    void* writefds;
    void* exceptfds;
    void* timeout;

    __u8 _pad[52 - 4 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_select);

struct mess_vfs_link {
    void* old_path;
    void* new_path;
    size_t old_path_len;
    size_t new_path_len;

    __u8 _pad[56 - 2 * sizeof(void*) - 2 * sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_link);

struct mess_vfs_fdpair {
    int retval;
    int fd0;
    int fd1;

    __u8 _pad[44];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_fdpair);

struct mess_vfs_poll {
    void* fds;
    unsigned int nfds;
    int timeout_msecs;

    __u8 _pad[48 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_poll);

struct mess_vfs_signalfd {
    int fd;
    unsigned int mask;
    int flags;

    __u8 _pad[44];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_signalfd);

struct mess_vfs_timerfd {
    int clock_id;
    int flags;
    int fd;

    void* new_value;
    void* old_value;

    __u8 _pad[44 - 2 * sizeof(void*)];
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

    __u8 _pad[32 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_epoll);

struct mess_vfs_mapdriver {
    void* label;
    int label_len;
    int* domains;
    int nr_domains;

    __u8 _pad[48 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_mapdriver);

BEGIN_MESS_DECL(mess_vfs_socket)
{
    int domain;
    int type;
    int protocol;

    __u8 _pad[44];
}
END_MESS_DECL(mess_vfs_socket)

struct mess_vfs_fs_symlink {
    dev_t dev;
    ino_t dir_ino;
    void* name;
    size_t name_len;
    __endpoint_t src;
    void* target;
    size_t target_len;
    uid_t uid;
    gid_t gid;

    __u8 _pad[56 - sizeof(dev_t) - sizeof(ino_t) - 2 * sizeof(void*) -
              2 * sizeof(size_t) - sizeof(__endpoint_t) - sizeof(uid_t) -
              sizeof(gid_t)];
} __attribute((packed));
VERIFY_MESS_SIZE(mess_vfs_fs_symlink);

BEGIN_MESS_DECL(mess_vfs_fs_create)
{
    dev_t dev;
    ino_t num;
    uid_t uid;
    gid_t gid;
    __mgrant_id_t grant;
    int name_len;
    mode_t mode;

    __u8 _pad[48 - sizeof(dev_t) - sizeof(ino_t) - sizeof(uid_t) -
              sizeof(gid_t) - sizeof(mode_t)];
}
END_MESS_DECL(mess_vfs_fs_create)

BEGIN_MESS_DECL(mess_vfs_fs_create_reply)
{
    int status;
    ino_t num;
    uid_t uid;
    gid_t gid;
    mode_t mode;
    size_t size;

    __u8 _pad[52 - sizeof(ino_t) - sizeof(uid_t) - sizeof(gid_t) -
              sizeof(size_t) - sizeof(mode_t)];
}
END_MESS_DECL(mess_vfs_fs_create_reply)

struct mess_vfs_cdev_openclose {
    __u64 minor;
    __u32 id;

    __u8 _pad[44];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_openclose);

struct mess_vfs_cdev_readwrite {
    __u64 minor;
    __endpoint_t endpoint;
    __u32 id;
    void* buf;
    __u32 request;
    off_t pos;
    size_t count;

    __u8 _pad[36 - sizeof(void*) - sizeof(off_t) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_readwrite);

struct mess_vfs_cdev_mmap {
    __u64 minor;
    __u32 id;
    void* addr;
    __endpoint_t endpoint;
    off_t pos;
    size_t count;

    __u8 _pad[40 - sizeof(void*) - sizeof(off_t) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_mmap);

struct mess_vfs_cdev_select {
    __u64 minor;
    __u32 ops;
    __u32 id;

    __u8 _pad[40];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_select);

struct mess_vfs_cdev_reply {
    __s32 status;
    __u32 id;
    void* retaddr;

    __u8 _pad[48 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_reply);

struct mess_vfs_cdev_poll_notify {
    __u64 minor;
    __u32 status;

    __u8 _pad[44];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_cdev_poll_notify);

struct mess_vfs_pm_signalfd {
    __endpoint_t endpoint;
    unsigned int sigmask;
    void* buf;

    __u8 _pad[48 - sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_vfs_pm_signalfd);

struct mess_pm_vfs_signalfd_reply {
    int status;
    __endpoint_t endpoint;

    __u8 _pad[48];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_pm_vfs_signalfd_reply);

struct mess_sysfs_publish_link {
    void* target;
    void* link_path;
    size_t target_len;
    size_t link_path_len;

    __u8 _pad[56 - 2 * sizeof(void*) - 2 * sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_sysfs_publish_link);

struct mess_bdev_blockdriver_msg {
    int minor;

    __u64 pos;
    size_t count;
    void* buf;

    __endpoint_t endpoint;
    int request;

    __u8 _pad[36 - sizeof(void*) - sizeof(size_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_bdev_blockdriver_msg);

struct mess_blockdriver_bdev_reply {
    ssize_t status;

    __u8 _pad[56 - sizeof(ssize_t)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_blockdriver_bdev_reply);

struct mess_inputdriver_input_event {
    int id;
    __u16 type;
    __u16 code;
    __s32 value;

    __u8 _pad[44];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_inputdriver_input_event);

struct mess_inputdriver_register_device {
    int dev_id;
    void* dev_bits;
    void* input_id;

    __u8 _pad[52 - 2 * sizeof(void*)];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_inputdriver_register_device);

struct mess_input_tty_event {
    int id;
    __u16 type;
    __u16 code;
    __s32 value;

    __u8 _pad[44];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_input_tty_event);

struct mess_input_conf {
    int status;
    int id;

    __u8 _pad[48];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_input_conf);

struct mess_devman_register_reply {
    int status;
    int id;

    __u8 _pad[48];
} __attribute__((packed));
VERIFY_MESS_SIZE(mess_devman_register_reply);

BEGIN_MESS_DECL(mess_sockdriver_socket)
{
    int req_id;
    __endpoint_t endpoint;
    int domain;
    int type;
    int protocol;

    __u8 _pad[36];
}
END_MESS_DECL(mess_sockdriver_socket)

BEGIN_MESS_DECL(mess_sockdriver_socket_reply)
{
    int req_id;
    int status;
    int sock_id;
    int sock_id2;

    __u8 _pad[40];
}
END_MESS_DECL(mess_sockdriver_socket_reply)

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
        struct mess_mm_mmap m_mm_mmap;
        struct mess_mm_mmap_reply m_mm_mmap_reply;
        struct mess_mm_remap m_mm_remap;
        struct mess_pm_signal m_pm_signal;
        struct mess_pm_clone m_pm_clone;
        struct mess_vfs_select m_vfs_select;
        struct mess_vfs_link m_vfs_link;
        struct mess_vfs_fdpair m_vfs_fdpair;
        struct mess_vfs_poll m_vfs_poll;
        struct mess_vfs_signalfd m_vfs_signalfd;
        struct mess_vfs_timerfd m_vfs_timerfd;
        struct mess_vfs_epoll m_vfs_epoll;
        struct mess_vfs_mapdriver m_vfs_mapdriver;
        struct mess_vfs_socket m_vfs_socket;
        struct mess_vfs_fs_symlink m_vfs_fs_symlink;
        struct mess_vfs_fs_create m_vfs_fs_create;
        struct mess_vfs_fs_create_reply m_vfs_fs_create_reply;
        struct mess_vfs_cdev_openclose m_vfs_cdev_openclose;
        struct mess_vfs_cdev_readwrite m_vfs_cdev_readwrite;
        struct mess_vfs_cdev_mmap m_vfs_cdev_mmap;
        struct mess_vfs_cdev_select m_vfs_cdev_select;
        struct mess_vfs_cdev_reply m_vfs_cdev_reply;
        struct mess_vfs_cdev_poll_notify m_vfs_cdev_poll_notify;
        struct mess_vfs_pm_signalfd m_vfs_pm_signalfd;
        struct mess_pm_vfs_signalfd_reply m_pm_vfs_signalfd_reply;
        struct mess_sysfs_publish_link m_sysfs_publish_link;
        struct mess_bdev_blockdriver_msg m_bdev_blockdriver_msg;
        struct mess_blockdriver_bdev_reply m_blockdriver_bdev_reply;
        struct mess_inputdriver_register_device m_inputdriver_register_device;
        struct mess_inputdriver_input_event m_inputdriver_input_event;
        struct mess_input_tty_event m_input_tty_event;
        struct mess_input_conf m_input_conf;
        struct mess_devman_register_reply m_devman_register_reply;
        struct mess_sockdriver_socket m_sockdriver_socket;
        struct mess_sockdriver_socket_reply m_sockdriver_socket_reply;

        __u8 _pad[56];
    } u;
} __attribute__((packed)) MESSAGE;
typedef int _VERIFY_MESSAGE[sizeof(MESSAGE) == 64 ? 1 : -1];

int send_recv(int function, int src_dest, MESSAGE* msg);

#undef VERIFY_MESS_SIZE
#undef BEGIN_MESS_DECL
#undef END_MESS_DECL

#endif
