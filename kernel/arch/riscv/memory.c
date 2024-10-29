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

#include <lyos/types.h>
#include <lyos/ipc.h>
#include "sys/types.h"
#include "stdio.h"
#include "unistd.h"
#include "stddef.h"
#include "lyos/const.h"
#include "string.h"
#include <kernel/proc.h>
#include <kernel/global.h>
#include <kernel/proto.h>
#include <asm/const.h>
#include <asm/proto.h>
#ifdef CONFIG_SMP
#include <asm/smp.h>
#endif
#include "asm/cpulocals.h"
#include <lyos/param.h>
#include <lyos/vm.h>
#include <errno.h>
#include <lyos/smp.h>
#include <asm/pagetable.h>

extern int _KERN_OFFSET;

/* Temporary mappings */
#define MAX_TEMPPDES 2
#define TEMPPDE_SRC  0
#define TEMPPDE_DST  1
static int temppdes[MAX_TEMPPDES];

#define _SRC_       0
#define _DEST_      1
#define EFAULT_SRC  1
#define EFAULT_DEST 2

void init_memory()
{
    int i;

    for (i = 0; i < MAX_TEMPPDES; i++) {
        temppdes[i] = ARCH_VM_DIR_ENTRIES - MAX_TEMPPDES + i;
    }

    get_cpulocal_var(pt_proc) = proc_addr(TASK_MM);
}

void clear_memcache()
{
    int i;

    for (i = 0; i < MAX_TEMPPDES; i++) {
        struct proc* p = get_cpulocal_var(pt_proc);
        int pde = temppdes[i];
        p->seg.ptbr_vir[pde] = 0;
    }
}

/* Temporarily map la in p's address space in kernel address space */
static void* create_temp_map(struct proc* p, void* la, size_t* len, int index,
                             int* changed)
{
    phys_bytes pa;
    pde_t pdeval;
    unsigned long pde = temppdes[index];

    /* the process is already in current page table */
    if (p && (p == get_cpulocal_var(pt_proc) || is_kerntaske(p->endpoint)))
        return la;

    if (p) {
        if (!p->seg.ptbr_vir) panic("create_temp_map: proc ptbr_vir not set");
        pdeval = __pde(p->seg.ptbr_vir[ARCH_PDE(la)]);
    } else {
        pa = (phys_bytes)la;
        return __va(pa);
    }

    if (!get_cpulocal_var(pt_proc)->seg.ptbr_vir)
        panic("create_temp_map: pt_proc ptbr_vir not set");
    if (get_cpulocal_var(pt_proc)->seg.ptbr_vir[pde] != pde_val(pdeval)) {
        get_cpulocal_var(pt_proc)->seg.ptbr_vir[pde] = pde_val(pdeval);
        *changed = 1;
    }

    unsigned long offset = ((uintptr_t)la) % ARCH_PGD_SIZE;
    *len = min(*len, ARCH_PGD_SIZE - offset);

    return (void*)(uintptr_t)(-((ARCH_VM_DIR_ENTRIES - pde) << ARCH_PGD_SHIFT) +
                              offset);
}

int la_la_copy(struct proc* p_dest, void* dest_la, struct proc* p_src,
               void* src_la, size_t len)
{
    int retval = 0;

    if (!get_cpulocal_var(pt_proc)) panic("pt_proc not present");
    if (read_ptbr() != get_cpulocal_var(pt_proc)->seg.ptbr_phys)
        panic("bad pt_proc ptbr value");

    enable_user_access();
    while (len > 0) {
        size_t chunk = len;
        void *src_mapped, *dest_mapped;
        int changed = 0;

        src_mapped =
            create_temp_map(p_src, src_la, &chunk, TEMPPDE_SRC, &changed);
        dest_mapped =
            create_temp_map(p_dest, dest_la, &chunk, TEMPPDE_DST, &changed);

        if (changed) flush_tlb();

        void* fault_addr = phys_copy(dest_mapped, src_mapped, chunk);

        if (fault_addr) {
            retval = EFAULT_SRC;
            if (fault_addr >= src_mapped && fault_addr < src_mapped + chunk)
                goto out;
            retval = EFAULT_DEST;
            if (fault_addr >= dest_mapped && fault_addr < dest_mapped + chunk)
                goto out;
            retval = EFAULT;
            goto out;
        }

        len -= chunk;
        src_la += chunk;
        dest_la += chunk;
    }

out:
    disable_user_access();
    return retval;
}

phys_bytes va2pa(endpoint_t ep, void* va)
{
    pde_t* pde;
    pmd_t* pmde;
    pte_t* pte;
    int slot;
    struct proc* p;

    if (is_kerntaske(ep)) return __pa(va);

    if (!verify_endpt(ep, &slot)) panic("va2pa: invalid endpoint");
    p = proc_addr(slot);

    pde = pgd_offset(p->seg.ptbr_vir, (unsigned long)va);
    if (pde_none(*pde)) {
        return -1;
    }

    pmde = pmd_offset(pde, (unsigned long)va);
    if (pmde_none(*pmde)) {
        return -1;
    }

    pte = pte_offset(pmde, (unsigned long)va);
    if (!pte_present(*pte)) {
        return -1;
    }

    return (pte_pfn(*pte) << ARCH_PG_SHIFT) + ((uintptr_t)va % ARCH_PG_SIZE);
}

#define MAX_KERN_MAPPINGS 8

struct kern_map {
    phys_bytes phys_addr;
    phys_bytes len;
    int flags;
    void* vir_addr;
    void** mapped_addr;
    void (*callback)(void*);
    void* cb_arg;
};

static struct kern_map kern_mappings[MAX_KERN_MAPPINGS];
static int kern_mapping_count = 0;

int kern_map_phys(phys_bytes phys_addr, phys_bytes len, int flags,
                  void** mapped_addr, void (*callback)(void*), void* arg)
{
    if (kern_mapping_count >= MAX_KERN_MAPPINGS) return ENOMEM;

    struct kern_map* pkm = &kern_mappings[kern_mapping_count++];
    pkm->phys_addr = phys_addr;
    pkm->len = len;
    pkm->flags = flags;
    pkm->mapped_addr = mapped_addr;
    pkm->callback = callback;
    pkm->cb_arg = arg;

    return 0;
}

#define KM_USERMAPPED   0
#define KM_KERN_MAPPING 1

extern char _usermapped[], _eusermapped[];
off_t usermapped_offset;

int arch_get_kern_mapping(int index, caddr_t* addr, int* len, int* flags)
{
    if (index >= KM_KERN_MAPPING + kern_mapping_count) return 1;

    if (index == KM_USERMAPPED) {
        *addr = (caddr_t)__pa((char*)*(&_usermapped));
        *len = (char*)*(&_eusermapped) - (char*)*(&_usermapped);
        *flags = KMF_USER | KMF_EXEC;
        return 0;
    }

    if (index >= KM_KERN_MAPPING &&
        index < KM_KERN_MAPPING + kern_mapping_count) {
        struct kern_map* pkm = &kern_mappings[index - KM_KERN_MAPPING];
        *addr = (caddr_t)pkm->phys_addr;
        *len = pkm->len;
        *flags = pkm->flags;
        return 0;
    }

    return 0;
}

void syscall_scall();

int arch_reply_kern_mapping(int index, void* vir_addr)
{
    char* usermapped_start = (char*)*(&_usermapped);

#define USER_PTR(x) (((char*)(x) - usermapped_start) + (char*)vir_addr)

    if (index == KM_USERMAPPED) {
        usermapped_offset = (char*)vir_addr - usermapped_start;
        sysinfo.user_info.magic = SYSINFO_MAGIC;
        sysinfo_user = (struct sysinfo*)USER_PTR(&sysinfo);
        sysinfo.kinfo = (kinfo_t*)USER_PTR(&kinfo);
        sysinfo.kern_log = (struct kern_log*)USER_PTR(&kern_log);
        sysinfo.machine = (struct machine*)USER_PTR(&machine);
        sysinfo.user_info.clock_info =
            (struct kclockinfo*)USER_PTR(&kclockinfo);
        sysinfo.user_info.syscall_gate =
            (syscall_gate_t)USER_PTR(syscall_scall);

        return 0;
    }

    if (index >= KM_KERN_MAPPING &&
        index < KM_KERN_MAPPING + kern_mapping_count) {
        kern_mappings[index - KM_KERN_MAPPING].vir_addr = vir_addr;
        return 0;
    }

    return 0;
}

static void setptbr(struct proc* p, phys_bytes ptbr)
{
    p->seg.ptbr_phys = (reg_t)ptbr;
    p->seg.ptbr_vir = (reg_t)__va(ptbr);

    if (p->endpoint == TASK_MM) {
        int i;

        write_ptbr(ptbr);
        get_cpulocal_var(pt_proc) = proc_addr(TASK_MM);

        /* update mapped address of kernel mappings */
        for (i = 0; i < kern_mapping_count; i++) {
            struct kern_map* pkm = &kern_mappings[i];
            *pkm->mapped_addr = pkm->vir_addr;
            if (pkm->callback) pkm->callback(pkm->cb_arg);
        }

        plic_enable(cpuid);

        smp_commence();
    }

    PST_UNSET(p, PST_MMINHIBIT);
}

int arch_vmctl(MESSAGE* m, struct proc* p)
{
    int request = m->VMCTL_REQUEST;

    switch (request) {
    case VMCTL_GETPDBR:
        m->VMCTL_PHYS_ADDR = (unsigned long)p->seg.ptbr_phys;
        return 0;
    case VMCTL_SET_ADDRESS_SPACE:
        setptbr(p, (phys_bytes)m->VMCTL_PHYS_ADDR);
        return 0;
    case VMCTL_FLUSHTLB:
        flush_tlb();
        return 0;
    }

    return EINVAL;
}

void mm_suspend(struct proc* caller, endpoint_t target, void* laddr,
                size_t bytes, int write, int type)
{
    PST_SET_LOCKED(caller, PST_MMREQUEST);

    caller->mm_request.req_type = MMREQ_CHECK;
    caller->mm_request.type = type;
    caller->mm_request.target = target;
    caller->mm_request.params.check.start = laddr;
    caller->mm_request.params.check.len = bytes;
    caller->mm_request.params.check.write = write;

    caller->mm_request.next_request = mmrequest;
    mmrequest = caller;
    if (!caller->mm_request.next_request) {
        if (send_sig(TASK_MM, SIGKMEM) != 0)
            panic("mm_suspend: send_sig failed");
    }
}

int _vir_copy(struct proc* caller, struct vir_addr* dest_addr,
              struct vir_addr* src_addr, size_t bytes, int check)
{
    struct vir_addr* vir_addrs[2];
    struct proc* procs[2];

    if (bytes < 0) return EINVAL;

    vir_addrs[_SRC_] = src_addr;
    vir_addrs[_DEST_] = dest_addr;

    int i;
    for (i = _SRC_; i <= _DEST_; i++) {
        endpoint_t proc_ep = vir_addrs[i]->proc_ep;

        procs[i] = proc_ep == NO_TASK ? NULL : endpt_proc(proc_ep);

        if (proc_ep != NO_TASK && procs[i] == NULL) return ESRCH;
    }

    int retval = la_la_copy(procs[_DEST_], vir_addrs[_DEST_]->addr,
                            procs[_SRC_], vir_addrs[_SRC_]->addr, bytes);

    if (retval) {
        if (retval == EFAULT) return EFAULT;

        if (retval != EFAULT_SRC && retval != EFAULT_DEST)
            panic("vir_copy: la_la_copy failed");

        if (!check || !caller) return EFAULT;

        int write;
        void* fault_la;
        endpoint_t target;
        if (retval == EFAULT_SRC) {
            target = vir_addrs[_SRC_]->proc_ep;
            fault_la = vir_addrs[_SRC_]->addr;
            write = 0;
        } else if (retval == EFAULT_DEST) {
            target = vir_addrs[_DEST_]->proc_ep;
            fault_la = vir_addrs[_DEST_]->addr;
            write = 1;
        }

        mm_suspend(caller, target, fault_la, bytes, write, MMREQ_TYPE_SYSCALL);
        return MMSUSPEND;
    }

    return 0;
}

int _data_vir_copy(struct proc* caller, endpoint_t dest_ep, void* dest_addr,
                   endpoint_t src_ep, void* src_addr, int len, int check)
{
    struct vir_addr src, dest;

    src.addr = src_addr;
    src.proc_ep = src_ep;

    dest.addr = dest_addr;
    dest.proc_ep = dest_ep;

    if (check)
        return vir_copy_check(caller, &dest, &src, len);
    else
        return vir_copy(&dest, &src, len);
}
