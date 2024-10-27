#include <lyos/ipc.h>
#include <stdio.h>
#include <errno.h>
#include <lyos/const.h>
#include <lyos/sysutils.h>
#include <libfsdriver/libfsdriver.h>

#include <libcoro/libcoro.h>
#include <libblockdriver/libblockdriver.h>
#include <libasyncdriver/libasyncdriver.h>

#include "proto.h"

static const struct fsdriver* fsdrv = NULL;

static void fsd_process(MESSAGE* msg);

static struct asyncdriver async_drv = {
    .name = "fsdriver_async",
    .process = fsd_process,
};

static void fsd_process(MESSAGE* msg) { fsdriver_process(fsdrv, msg); }

int fsdriver_async_start(const struct fsdriver* fsd, size_t num_workers,
                         void (*init_func)(void))
{
    int retval;

    fsdrv = fsd;

    retval = fsdriver_register(fsd);
    if (retval != 0) return retval;

    if (fsd->name) async_drv.name = fsd->name;

    asyncdrv_task(&async_drv, num_workers, init_func);

    return 0;
}
