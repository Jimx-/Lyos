file /home/jimx/projects/lyos/arch/arm64/lyos.elf
# file /home/jimx/projects/lyos/obj/destdir.x86_64/lib/ld-lyos.so
# file /home/jimx/projects/lyos/obj/destdir.x86_64/sbin/ext2fs
target remote 127.0.0.1:1234
# set disassembly-flavor intel
# set archi i386
# hb ldso_main
layout asm
hb __primary_switched
