#ifndef _LIBASYNCDRIVER_WORK_H_
#define _LIBASYNCDRIVER_WORK_H_

#include <lyos/list.h>

struct async_work;

typedef void (*async_work_func_t)(struct async_work* work);

struct async_work {
    unsigned int flags;
#define ASYNC_WORK_PENDING 0x01

    struct list_head entry;
    async_work_func_t func;
};

#define INIT_ASYNC_WORK(_work, _func)    \
    do {                                 \
        (_work)->flags = 0;              \
        INIT_LIST_HEAD(&(_work)->entry); \
        (_work)->func = (_func);         \
    } while (0)

int asyncdrv_enqueue_work(struct async_work* work);

#endif
