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

#ifndef _VFS_PROTO_H_
#define _VFS_PROTO_H_

#include "path.h"
#include "thread.h"

int vfs_verify_endpt(endpoint_t ep, int* proc_nr);
struct fproc* vfs_endpt_proc(endpoint_t ep);

int do_register_filesystem();
int add_filesystem(endpoint_t fs_ep, char* name);
endpoint_t get_filesystem_endpoint(char* name);

void init_inode_table();
struct inode* new_inode(dev_t dev, ino_t num);
struct inode* find_inode(dev_t dev, ino_t num);
void put_inode(struct inode* pin);
int lock_inode(struct inode* pin, rwlock_type_t type);
void unlock_inode(struct inode* pin);
void sync_inode(struct inode* p);

void init_lookup(struct lookup* lookup, char* pathname, int flags,
                 struct vfs_mount** vmnt, struct inode** inode);
struct inode* resolve_path(struct lookup* lookup, struct fproc* fp);
struct inode* advance_path(struct inode* start, struct lookup* lookup,
                           struct fproc* fp);
struct inode* last_dir(struct lookup* lookup, struct fproc* fp);

struct vfs_mount* find_vfs_mount(dev_t dev);
int lock_vmnt(struct vfs_mount* vmnt, rwlock_type_t type);
void unlock_vmnt(struct vfs_mount* vmnt);
int mount_fs(dev_t dev, char* mountpoint, endpoint_t fs_ep, int readonly);
int forbidden(struct fproc* fp, struct inode* pin, int access);
mode_t do_umask(void);
void clear_vfs_mount(struct vfs_mount* vmnt);
struct vfs_mount* get_free_vfs_mount();
int do_vfs_open(MESSAGE* p);

int request_put_inode(endpoint_t fs_e, dev_t dev, ino_t num);
int request_lookup(endpoint_t fs_e, char* pathname, dev_t dev, ino_t start,
                   ino_t root, struct fproc* fp, struct lookup_result* ret);
int request_readsuper(endpoint_t fs_ep, dev_t dev, int readonly, int is_root,
                      struct lookup_result* res);

int do_open(void);
int common_open(char* pathname, int flags, mode_t mode);
int do_close(void);
int close_fd(struct fproc* fp, int fd);
int do_lseek(void);
int do_chroot(MESSAGE* p);
int do_mount(void);
int do_umount(MESSAGE* p);
int do_mkdir(void);

/* fs/Lyos/read_write.c */
int do_rdwt(void);
int do_getdents(void);

/* fs/Lyos/link.c */
int do_unlink(MESSAGE* p);
int do_rdlink(void);
int truncate_node(struct inode* pin, int newsize);

int do_dup(void);
int do_chdir(void);
int do_fchdir(void);

int do_mm_request(void);
int fs_exec(void);

int request_stat(endpoint_t fs_ep, dev_t dev, ino_t num, int src, char* buf);

int request_readwrite(endpoint_t fs_ep, dev_t dev, ino_t num, u64 pos,
                      int rw_flag, endpoint_t src, void* buf, size_t nbytes,
                      u64* newpos, size_t* bytes_rdwt);

int do_stat(void);
int do_fstat(void);
int do_access(void);
int do_chmod(int type);
int fs_getsetid(void);

int do_ioctl(void);
int do_fcntl(void);

int do_sync(void);

int fs_fork(void);
int fs_exit(void);

/* worker.c */
int rwlock_lock(rwlock_t* rwlock, rwlock_type_t lock_type);

void worker_init(void);
void worker_yield(void);
void worker_wait(void);
void worker_wake(struct worker_thread* worker);
void worker_dispatch(struct fproc* fp, void (*func)(void), MESSAGE* msg);
void worker_allow(int allow);
struct worker_thread* worker_suspend(void);
void worker_resume(struct worker_thread* worker);
void revive_proc(endpoint_t endpoint, MESSAGE* msg);
struct worker_thread* worker_get(thread_t tid);

/* ipc.c */
int fs_sendrec(endpoint_t fs_e, MESSAGE* msg);

void lock_filp(struct file_desc* filp, rwlock_type_t lock_type);
void unlock_filp(struct file_desc* filp);
struct file_desc* get_filp(struct fproc* fp, int fd, rwlock_type_t lock_type);
int get_fd(struct fproc* fp, int start, int* fd, struct file_desc** fpp);

int cdev_open(dev_t dev);
int cdev_close(dev_t dev);
int cdev_io(int op, dev_t dev, endpoint_t src, void* buf, off_t pos,
            size_t count, struct fproc* fp);
int cdev_mmap(dev_t dev, endpoint_t src, void* vaddr, off_t offset,
              size_t length, struct fproc* fp);
int cdev_select(dev_t dev, int ops, struct fproc* fp);
int cdev_reply(MESSAGE* msg);
struct cdmap* cdev_lookup_by_endpoint(endpoint_t driver_ep);

void init_select(void);
int do_select(void);
void do_select_cdev_reply1(endpoint_t driver_ep, dev_t minor, int status);
void do_select_cdev_reply2(endpoint_t driver_ep, dev_t minor, int status);

#endif
