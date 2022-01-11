#ifndef _NET_IF_H
#define _NET_IF_H

#include <sys/cdefs.h>
#include <sys/socket.h>

#define IF_NAMESIZE 16
#define IFNAMSIZ    IF_NAMESIZE

struct if_nameindex {
    unsigned int if_index;
    char* if_name;
};

struct ifmap {
    unsigned long mem_start;
    unsigned long mem_end;
    unsigned short base_addr;
    unsigned char irq;
    unsigned char dma;
    unsigned char port;
};

struct ifreq {
    char ifr_name[IFNAMSIZ];
    union {
        struct sockaddr ifr_addr;
        struct sockaddr ifr_dstaddr;
        struct sockaddr ifr_broadaddr;
        struct sockaddr ifr_netmask;
        struct sockaddr ifr_hwaddr;
        short ifr_flags;
        int ifr_ifindex;
        int ifr_metric;
        int ifr_mtu;
        struct ifmap ifr_map;
        char ifr_slave[IFNAMSIZ];
        char ifr_newname[IFNAMSIZ];
        char* ifr_data;
    };
};

struct ifconf {
    int ifc_len;
    union {
        char* ifc_buf;
        struct ifreq* ifc_req;
    };
};

#define IFF_UP          0x1
#define IFF_BROADCAST   0x2
#define IFF_DEBUG       0x4
#define IFF_LOOPBACK    0x8
#define IFF_POINTOPOINT 0x10
#define IFF_NOTRAILERS  0x20
#define IFF_RUNNING     0x40
#define IFF_PROMISC     0x100
#define IFF_MULTICAST   0x1000

__BEGIN_DECLS

void if_freenameindex(struct if_nameindex*);
char* if_indextoname(unsigned int, char*);
struct if_nameindex* if_nameindex(void);
unsigned int if_nametoindex(const char*);

__END_DECLS

#endif /* _NET_IF_H */
