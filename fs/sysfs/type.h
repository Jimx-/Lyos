#ifndef _SYSFS_TYPE_H_
#define _SYSFS_TYPE_H_

#include <stddef.h>

struct sysfs_buf {
    char* buf;
    size_t used, left;
    off_t offset;
};

#endif
