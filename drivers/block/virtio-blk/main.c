#include <lyos/types.h>
#include <lyos/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <lyos/const.h>
#include <string.h>
#include <lyos/driver.h>
#include <errno.h>
#include <lyos/portio.h>
#include <lyos/irqctl.h>
#include <lyos/service.h>
#include <lyos/sysutils.h>
#include <lyos/vm.h>
#include <sys/mman.h>

#include <libblockdriver/libblockdriver.h>
#include <libvirtio/libvirtio.h>
#include <libsysfs/libsysfs.h>

#include "virtio_blk.h"

#define MAX_THREADS 4

static const char* name = "virtio-blk";
static int instance;
static int open_count = 0;

static struct part_info primary[NR_PRIM_PER_DRIVE];
static struct part_info logical[NR_SUB_PER_DRIVE];

static struct virtio_device* vdev;
static struct virtqueue** vqs;
static unsigned int num_vqs;

struct virtio_blk_config blk_config;

static struct virtio_blk_outhdr* hdrs_vir;
static phys_bytes hdrs_phys;

#define mystatus(tid) (status_vir[(tid)] & 0xff)
static u16* status_vir;
static phys_bytes status_phys;

struct virtio_feature features[] = {{"barrier", VIRTIO_BLK_F_BARRIER, 0, 0},
                                    {"sizemax", VIRTIO_BLK_F_SIZE_MAX, 0, 0},
                                    {"segmax", VIRTIO_BLK_F_SEG_MAX, 0, 0},
                                    {"geometry", VIRTIO_BLK_F_GEOMETRY, 0, 0},
                                    {"read-only", VIRTIO_BLK_F_RO, 0, 0},
                                    {"blocksize", VIRTIO_BLK_F_BLK_SIZE, 0, 0},
                                    {"scsi", VIRTIO_BLK_F_SCSI, 0, 0},
                                    {"flush", VIRTIO_BLK_F_FLUSH, 0, 0},
                                    {"topology", VIRTIO_BLK_F_TOPOLOGY, 0, 0},
                                    {"idbytes", VIRTIO_BLK_ID_BYTES, 0, 0}};

static int virtio_blk_open(dev_t minor, int access);
static int virtio_blk_close(dev_t minor);
static ssize_t virtio_blk_rdwt(dev_t minor, int do_write, loff_t pos,
                               endpoint_t endpoint,
                               const struct iovec_grant* iov, size_t count);
static struct part_info* virtio_blk_part(dev_t device);
static void virtio_blk_other(MESSAGE* msg);
static void virtio_blk_intr(unsigned mask);
static int virtio_blk_status2error(u8 status);

static int virtio_blk_get_id(char* id_str);
static ssize_t virtio_blk_serial_show(struct device_attribute* attr, char* buf,
                                      size_t size);

static struct blockdriver virtio_blk_driver = {
    .bdr_open = virtio_blk_open,
    .bdr_close = virtio_blk_close,
    .bdr_readwrite = virtio_blk_rdwt,
    .bdr_part = virtio_blk_part,
    .bdr_intr = virtio_blk_intr,
    .bdr_other = virtio_blk_other,
};

static struct part_info* virtio_blk_part(dev_t device)
{
    int part_no = MINOR(device) % NR_SUB_PER_DRIVE;
    struct part_info* part = part_no < NR_PRIM_PER_DRIVE
                                 ? &primary[part_no]
                                 : &logical[part_no - NR_PRIM_PER_DRIVE];

    return part;
}

static int virtio_blk_open(dev_t minor, int access)
{
    open_count++;
    return 0;
}

static int virtio_blk_close(dev_t minor)
{
    open_count--;
    return 0;
}

static int fill_buffers(struct iovec_grant* iov, struct umap_phys* phys,
                        size_t count, endpoint_t endpoint, int do_write)
{
    int i, retval;

    for (i = 0; i < count; i++) {
        if (endpoint == SELF) {
            if ((retval = umap(endpoint, UMT_VADDR, (vir_bytes)iov[i].iov_addr,
                               iov[i].iov_len, &phys[i].phys_addr)) != 0)
                return -retval;
        } else {
            if ((retval = umap(endpoint, UMT_GRANT, iov[i].iov_grant,
                               iov[i].iov_len, &phys[i].phys_addr)) != 0)
                return -retval;
        }

        /* physical buffer */
        phys[i].phys_addr |= !do_write;
        phys[i].size = iov[i].iov_len;
    }

    return 0;
}

static ssize_t virtio_blk_rdwt(dev_t minor, int do_write, loff_t pos,
                               endpoint_t endpoint,
                               const struct iovec_grant* iov, size_t count)
{
    int i;
    struct part_info* part;
    u64 part_end, sector;
    struct iovec_grant virs[NR_IOREQS];
    struct umap_phys phys[NR_IOREQS + 2];
    blockdriver_worker_id_t tid;
    size_t size, tmp_size;
    int retval;

    if (count > NR_IOREQS) {
        return -EINVAL;
    }

    tid = blockdriver_async_worker_id();

    part = virtio_blk_part(minor);

    if (!part) {
        return -ENXIO;
    }

    pos += part->base;

    if (pos % blk_config.blk_size) {
        return -EINVAL;
    }

    part_end = part->base + part->size;

    if (pos >= part_end) {
        return 0;
    }

    size = 0;
    for (i = 0; i < count; i++) {
        size += iov[i].iov_len;
    }

    if (pos + size >= part_end) {
        size = part_end - pos;
        tmp_size = 0;
        count = 0;
    } else {
        tmp_size = size;
    }

    while (tmp_size < size) {
        tmp_size += iov[count++].iov_len;
    }

    for (i = 0; i < count; i++) {
        if (endpoint == SELF)
            virs[i].iov_addr = iov[i].iov_addr;
        else
            virs[i].iov_grant = iov[i].iov_grant;

        virs[i].iov_len = iov[i].iov_len;
    }

    if (tmp_size > size) {
        virs[count - 1].iov_len -= (tmp_size - size);
    }

    if (size % blk_config.blk_size) {
        return -EINVAL;
    }

    sector = pos / blk_config.blk_size;

    memset(&hdrs_vir[tid], 0, sizeof(*hdrs_vir));

    if (do_write) {
        hdrs_vir[tid].type = VIRTIO_BLK_T_OUT;
    } else {
        hdrs_vir[tid].type = VIRTIO_BLK_T_IN;
    }

    hdrs_vir[tid].ioprio = 0;
    hdrs_vir[tid].sector = sector;

    /* setup header */
    phys[0].phys_addr = hdrs_phys + tid * sizeof(*hdrs_vir);
    phys[0].size = sizeof(struct virtio_blk_outhdr);

    retval = fill_buffers(virs, &phys[1], count, endpoint, do_write);
    if (retval) return -retval;

    /* status */
    phys[count + 1].phys_addr = status_phys + tid * sizeof(*status_vir);
    phys[count + 1].phys_addr |= 1;
    phys[count + 1].size = sizeof(u8);

    virtqueue_add_buffers(vqs[0], phys, count + 2, (void*)(unsigned long)tid);

    virtqueue_kick(vqs[0]);

    blockdriver_async_sleep();

    if (mystatus(tid) == VIRTIO_BLK_S_OK) {
        return size;
    }

    return -virtio_blk_status2error(mystatus(tid));
}

static int virtio_blk_status2error(u8 status)
{
    switch (status) {
    case VIRTIO_BLK_S_IOERR:
        return EIO;
    case VIRTIO_BLK_S_UNSUPP:
        return ENOTSUP;
    default:
        panic("%s: unknown status %d", name, status);
    }

    return 0;
}

static void virtio_blk_intr(unsigned mask)
{
    void* data;
    blockdriver_worker_id_t tid;

    if (virtio_had_irq(vdev)) {
        while (!virtqueue_get_buffer(vqs[0], NULL, &data)) {
            tid = (blockdriver_worker_id_t)(unsigned long)data;

            blockdriver_async_wakeup(tid);
        }
    }

    virtio_enable_irq(vdev);
}

static int virtio_blk_get_id(char* id_str)
{
    struct umap_phys phys[3];
    blockdriver_worker_id_t tid;
    int retval;

    tid = blockdriver_async_worker_id();

    memset(&hdrs_vir[tid], 0, sizeof(*hdrs_vir));

    hdrs_vir[tid].type = VIRTIO_BLK_T_GET_ID;
    hdrs_vir[tid].ioprio = 0;
    hdrs_vir[tid].sector = 0;

    /* setup header */
    phys[0].phys_addr = hdrs_phys + tid * sizeof(*hdrs_vir);
    phys[0].size = sizeof(struct virtio_blk_outhdr);

    /* id string */
    if ((retval = umap(SELF, UMT_VADDR, (vir_bytes)id_str, VIRTIO_BLK_ID_BYTES,
                       &phys[1].phys_addr)) != 0) {
        return retval;
    }
    phys[1].phys_addr |= 1;
    phys[1].size = VIRTIO_BLK_ID_BYTES;

    /* status */
    phys[2].phys_addr = status_phys + tid * sizeof(*status_vir);
    phys[2].phys_addr |= 1;
    phys[2].size = sizeof(u8);

    virtqueue_add_buffers(vqs[0], phys, 3, (void*)(unsigned long)tid);

    virtqueue_kick(vqs[0]);

    blockdriver_async_sleep();

    if (mystatus(tid) == VIRTIO_BLK_S_OK) {
        return 0;
    }

    return virtio_blk_status2error(mystatus(tid));
}

static ssize_t virtio_blk_serial_show(struct device_attribute* attr, char* buf,
                                      size_t size)
{
    int retval;

    buf[VIRTIO_BLK_ID_BYTES] = 0;
    retval = virtio_blk_get_id(buf);

    if (retval == 0) {
        return strlen(buf);
    }

    if (retval == EIO) {
        return 0;
    }

    return -retval;
}

static int virtio_blk_init_vqs(void)
{
    int retval;

    num_vqs = 1;

    vqs = calloc(num_vqs, sizeof(*vqs));

    if (!vqs) {
        return ENOMEM;
    }

    retval = virtio_find_vqs(vdev, num_vqs, vqs);

    if (retval) {
        free(vqs);
    }

    return retval;
}

static int virtio_blk_alloc_requests(void)
{
    hdrs_vir =
        mmap(NULL, sizeof(*hdrs_vir) * MAX_THREADS, PROT_READ | PROT_WRITE,
             MAP_POPULATE | MAP_ANONYMOUS | MAP_CONTIG | MAP_PRIVATE, -1, 0);

    if (hdrs_vir == MAP_FAILED) {
        return ENOMEM;
    }

    if (umap(SELF, UMT_VADDR, (vir_bytes)hdrs_vir,
             sizeof(*hdrs_vir) * MAX_THREADS, &hdrs_phys) != 0) {
        panic("%s: umap failed", name);
    }

    status_vir =
        mmap(NULL, sizeof(*status_vir) * MAX_THREADS, PROT_READ | PROT_WRITE,
             MAP_POPULATE | MAP_ANONYMOUS | MAP_CONTIG | MAP_PRIVATE, -1, 0);

    if (status_vir == MAP_FAILED) {
        munmap(hdrs_vir, sizeof(*hdrs_vir) * MAX_THREADS);
        return ENOMEM;
    }

    if (umap(SELF, UMT_VADDR, (vir_bytes)status_vir,
             sizeof(*status_vir) * MAX_THREADS, &status_phys) != 0) {
        panic("%s: umap failed", name);
    }

    return 0;
}

static int virtio_blk_config(void)
{
    u32 size_mbs;

    printl("%s: Virtio block config:\n", name);

    virtio_cread(vdev, struct virtio_blk_config, capacity,
                 &blk_config.capacity);
    size_mbs = (blk_config.capacity << 9) >> 20;
    printl("  Capacity: %d MB\n", size_mbs);

    if (virtio_host_supports(vdev, VIRTIO_BLK_F_SEG_MAX)) {
        virtio_cread(vdev, struct virtio_blk_config, seg_max,
                     &blk_config.seg_max);

        printl("  Segment max: %d\n", blk_config.seg_max);
    }

    if (virtio_host_supports(vdev, VIRTIO_BLK_F_GEOMETRY)) {
        virtio_cread(vdev, struct virtio_blk_config, geometry.cylinders,
                     &blk_config.geometry.cylinders);
        virtio_cread(vdev, struct virtio_blk_config, geometry.heads,
                     &blk_config.geometry.heads);
        virtio_cread(vdev, struct virtio_blk_config, geometry.sectors,
                     &blk_config.geometry.sectors);

        printl("  Geometry: cylinders=%d heads=%d sectors=%d\n",
               blk_config.geometry.cylinders, blk_config.geometry.heads,
               blk_config.geometry.sectors);
    }

    if (virtio_host_supports(vdev, VIRTIO_BLK_F_BLK_SIZE)) {
        virtio_cread(vdev, struct virtio_blk_config, blk_size,
                     &blk_config.blk_size);

        printl("  Block size: %d\n", blk_config.blk_size);
    }

    return 0;
}

static int virtio_blk_probe(int instance)
{
    int retval;

    vdev =
        virtio_probe_device(0x0002, name, features,
                            sizeof(features) / sizeof(features[0]), instance);

    if (!vdev) return ENXIO;

    retval = virtio_blk_init_vqs();
    if (retval) {
        goto out_free_dev;
    }

    retval = virtio_blk_alloc_requests();
    if (retval) {
        goto out_free_vqs;
    }

    virtio_blk_config();

    virtio_device_ready(vdev);

    return 0;

out_free_vqs:
    vdev->config->del_vqs(vdev);
out_free_dev:
    virtio_free_device(vdev);
    vdev = NULL;

    return retval;
}

static void print_disk_info(void)
{
    int i;
    printl("Hard disk information:\n");
    for (i = 0; i < NR_PART_PER_DRIVE + 1; i++) {
        if (primary[i].size == 0) continue;

        u32 base_sect = primary[i].base >> SECTOR_SIZE_SHIFT;
        u32 nr_sects = primary[i].size >> SECTOR_SIZE_SHIFT;

        printl("%spartition %d: base %u(0x%x), size %u(0x%x) (in sectors)\n",
               i == 0 ? "  " : "     ", i, base_sect, base_sect, nr_sects,
               nr_sects);
    }
    for (i = 0; i < NR_SUB_PER_DRIVE; i++) {
        if (logical[i].size == 0) continue;

        u32 base_sect = logical[i].base >> SECTOR_SIZE_SHIFT;
        u32 nr_sects = logical[i].size >> SECTOR_SIZE_SHIFT;

        printl("              "
               "%d: base %d(0x%x), size %d(0x%x) (in sectors)\n",
               i, base_sect, base_sect, nr_sects, nr_sects);
    }
}

static void virtio_blk_register(void)
{
    int i;
    struct device_info devinf;
    struct device_attribute attr;
    dev_t devt;
    device_id_t dev_id;
    int retval;

    /* register the device */
    memset(&devinf, 0, sizeof(devinf));
    devinf.bus = NO_BUS_ID;
    devinf.class = NO_CLASS_ID;
    devinf.parent = vdev->dev_id;
    devinf.type = DT_BLOCKDEV;

    for (i = 0; i < NR_PART_PER_DRIVE + 1; i++) {
        if (primary[i].size == 0) continue;

        devt = MAKE_DEV(DEV_VD, i);
        dm_bdev_add(devt);

        snprintf(devinf.name, sizeof(devinf.name), "vd%d%c", instance,
                 'a' + (char)i);
        devinf.devt = devt;
        retval = blockdriver_device_register(&devinf, &dev_id);

        if (retval == 0) {
            dm_init_device_attr(&attr, dev_id, "serial", SF_PRIV_OVERWRITE,
                                NULL, virtio_blk_serial_show, NULL);
            dm_device_attr_add(&attr);
        }
    }

    for (i = 0; i < NR_SUB_PER_DRIVE; i++) {
        if (logical[i].size == 0) continue;

        devt = MAKE_DEV(DEV_VD, NR_PRIM_PER_DRIVE + i);
        dm_bdev_add(devt);

        snprintf(devinf.name, sizeof(devinf.name), "vd%d%c", instance,
                 'a' + (char)(NR_PRIM_PER_DRIVE + i));
        devinf.devt = devt;
        retval = blockdriver_device_register(&devinf, &dev_id);

        if (retval == 0) {
            dm_init_device_attr(&attr, dev_id, "serial", SF_PRIV_OVERWRITE,
                                NULL, virtio_blk_serial_show, NULL);
            dm_device_attr_add(&attr);
        }
    }
}

static int virtio_blk_init(void)
{
    int retval;

    instance = 0;

    printl("%s: Virtio block driver is running\n", name);

    retval = virtio_blk_probe(instance);

    if (retval == ENXIO) {
        panic("%s: no virtio-blk device found", name);
    }

    return 0;
}

static void virtio_blk_post_init(void)
{
    memset(primary, 0, sizeof(primary));
    memset(logical, 0, sizeof(logical));
    primary[0].size = blk_config.capacity * blk_config.blk_size;

    partition(&virtio_blk_driver, 0, P_PRIMARY);
    print_disk_info();
    virtio_blk_register();
}

static void virtio_blk_other(MESSAGE* msg)
{
    switch (msg->type) {
    case DM_BUS_ATTR_SHOW:
    case DM_BUS_ATTR_STORE:
        msg->CNT = dm_bus_attr_handle(msg);
        break;
    case DM_DEVICE_ATTR_SHOW:
    case DM_DEVICE_ATTR_STORE:
        dm_device_attr_handle(msg);
        msg->RETVAL = SUSPEND;
        break;
    default:
        msg->RETVAL = ENOSYS;
        break;
    }

    if (msg->RETVAL != SUSPEND) {
        msg->type = SYSCALL_RET;
        send_recv(SEND_NONBLOCK, msg->source, msg);
    }
}

int main()
{
    serv_register_init_fresh_callback(virtio_blk_init);
    serv_init();

    blockdriver_async_task(&virtio_blk_driver, MAX_THREADS,
                           virtio_blk_post_init);

    return 0;
}
