#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    echo -e "\033[1;31mYou're going to need to run this as root\033[0m" 1>&2
    echo "Additionally, verify that /dev/loop4 is available and that" 1>&2
    echo "/mnt is available for mounting; otherwise, modify the script" 1>&2
    echo "to use alternative loop devices or mount points as needed." 1>&2
    exit 1
fi

SUBARCH=$(uname -m | sed -e s/sun4u/sparc64/ \
        -e s/arm.*/arm/ -e s/sa110/arm/ \
        -e s/s390x/s390/ -e s/parisc64/parisc/ \
        -e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
        -e s/sh[234].*/sh/ )
ARCH=$SUBARCH

while getopts "m:" arg
do
        case $arg in
             m)
                ARCH=${OPTARG}
                ;;
             ?)
                echo "unkonw argument"
                exit 1
                ;;
        esac
done

if [ $ARCH = "i686" ]; then
    ARCH=x86
fi

if [ $ARCH = "x86_64" ]; then
    ARCH=x86
fi

if [ $ARCH = "riscv64" ]; then
    ARCH=riscv
fi

export SUBARCH=$SUBARCH ARCH=$ARCH

DISK=lyos-disk-$SUBARCH.img
SRCDIR=./
MOUNT_POINT=/mnt/lyos-root
DESTDIR=obj/destdir.$SUBARCH

source $SRCDIR/.config

LOOPRAW=`losetup -f`
losetup $LOOPRAW $DISK
TMP=`kpartx -av $DISK`
TMP2=${TMP/add map /}
LOOP=${TMP2%%p1 *}
LOOPDEV=/dev/${LOOP}
LOOPMAP=/dev/mapper/${LOOP}p1

mount $LOOPMAP /$MOUNT_POINT

echo "Installing kernel."
if [[ $CONFIG_COMPRESS_GZIP == "y" ]]
then
    cp -r $SRCDIR/arch/x86/lyos.gz /$MOUNT_POINT/boot/
else
    cp -r $SRCDIR/arch/x86/lyos.elf /$MOUNT_POINT/boot/
fi

#cp -rf $DESTDIR/boot/* /$MOUNT_POINT/boot/
#cp -rf $DESTDIR/bin/profile /$MOUNT_POINT/bin/
cp -rf $DESTDIR/usr/bin/bash /$MOUNT_POINT/usr/bin/bash
cp -rf $DESTDIR/bin/* /$MOUNT_POINT/bin/
#cp -rf $DESTDIR/sbin/procfs /$MOUNT_POINT/sbin/
cp -rf $DESTDIR/usr/bin/getty /$MOUNT_POINT/usr/bin/
cp -rf $DESTDIR/usr/bin/login /$MOUNT_POINT/usr/bin/
cp -rf $DESTDIR/usr/bin/vim /$MOUNT_POINT/usr/bin/
cp -rf $DESTDIR/usr/bin/strace /$MOUNT_POINT/usr/bin/
cp -rf $DESTDIR/usr/bin/lydm /$MOUNT_POINT/usr/bin/
cp -rf $DESTDIR/sbin/usbhid /$MOUNT_POINT/sbin/
#cp -rf $DESTDIR/usr/ /$MOUNT_POINT/
cp -rf $DESTDIR/usr/lib/libc.so /$MOUNT_POINT/usr/lib/
cp -rf $DESTDIR/lib/ld-lyos.so /$MOUNT_POINT/lib/
cp -rf $DESTDIR/usr/lib/libEGL.so.1.0.0 /$MOUNT_POINT/usr/lib/
cp -rf $DESTDIR/usr/lib/libgbm.so.1.0.0 /$MOUNT_POINT/usr/lib/
cp -rf $DESTDIR/usr/lib/dri/* /$MOUNT_POINT/usr/lib/dri/
cp -rf sysroot/etc/* /$MOUNT_POINT/etc/
cp -rf sysroot/usr/lib/udev/rules.d/* /$MOUNT_POINT/usr/lib/udev/rules.d/
# cp -rf /$MOUNT_POINT/home/jimx/profile.out .
#cp -rf sysroot/boot/* /$MOUNT_POINT/boot/
sync

umount $MOUNT_POINT
kpartx -d $LOOPMAP
dmsetup remove $LOOPMAP
losetup -d $LOOPDEV
losetup -d $LOOPRAW

if [ -n "$SUDO_USER" ] ; then
    echo "Reassigning permissions on disk image to $SUDO_USER"
    chown $SUDO_USER:$SUDO_USER $DISK
fi

echo "Done. You can boot the disk image with qemu now."
