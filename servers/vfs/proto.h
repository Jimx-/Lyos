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

PUBLIC int vfs_verify_endpt(endpoint_t ep, int * proc_nr);
PUBLIC struct fproc * vfs_endpt_proc(endpoint_t ep);

PUBLIC int do_register_filesystem(MESSAGE * p);
PUBLIC int add_filesystem(endpoint_t fs_ep, char * name);
PUBLIC endpoint_t get_filesystem_endpoint(char * name);

PUBLIC void init_inode_table();
PUBLIC struct inode * new_inode(dev_t dev, ino_t num);
PUBLIC struct inode * find_inode(dev_t dev, ino_t num);
PUBLIC void put_inode(struct inode * pin);
PUBLIC void lock_inode(struct inode * pin);
PUBLIC void unlock_inode(struct inode * pin);
PUBLIC void sync_inode  (struct inode * p);

PUBLIC struct inode * resolve_path(char * pathname, struct fproc * fp);
PUBLIC struct inode * last_dir(char * pathname, struct fproc * fp);

PUBLIC struct vfs_mount * find_vfs_mount(dev_t dev);
PUBLIC void lock_vmnt(struct vfs_mount * vmnt);
PUBLIC void unlock_vmnt(struct vfs_mount * vmnt);
PUBLIC int mount_fs(dev_t dev, char * mountpoint, endpoint_t fs_ep, int readonly);
PUBLIC int forbidden(struct fproc * fp, struct inode * pin, int access);
PUBLIC mode_t do_umask(MESSAGE * p);

PUBLIC int request_put_inode(endpoint_t fs_e, dev_t dev, ino_t num);
PUBLIC int request_lookup(endpoint_t fs_e, char * pathname, dev_t dev, 
                ino_t start, ino_t root, struct lookup_result * ret);
PUBLIC int request_readsuper(endpoint_t fs_ep, dev_t dev,
        int readonly, int is_root, struct lookup_result * res);

PUBLIC int do_open(MESSAGE * p);
PUBLIC int  do_close(MESSAGE * p);
PUBLIC int  do_lseek(MESSAGE * p);
PUBLIC int  do_chroot(MESSAGE * p);
PUBLIC int  do_mount(MESSAGE * p);
PUBLIC int  do_umount(MESSAGE * p);
PUBLIC int  do_mkdir(MESSAGE * p);

/* fs/Lyos/read_write.c */
PUBLIC int  do_rdwt(MESSAGE * p);

/* fs/Lyos/link.c */
PUBLIC int  do_unlink(MESSAGE * p);

PUBLIC int truncate_node(struct inode * pin, int newsize);

PUBLIC int do_dup(MESSAGE * p);
PUBLIC int do_chdir(MESSAGE * p);
PUBLIC int do_fchdir(MESSAGE * p);

PUBLIC int do_exec(MESSAGE * msg);

PUBLIC int request_stat(endpoint_t fs_ep, dev_t dev, ino_t num, int src, char * buf);

PUBLIC int request_readwrite(endpoint_t fs_ep, dev_t dev, ino_t num, u64 pos, int rw_flag, endpoint_t src,
    void * buf, int nbytes, u64 * newpos, int * bytes_rdwt);

PUBLIC int       do_stat(MESSAGE * p);
PUBLIC int       do_fstat(MESSAGE * p);
PUBLIC int       do_access(MESSAGE * p);

PUBLIC int do_ioctl(MESSAGE * p);
PUBLIC int do_fcntl(MESSAGE * p);

#endif
