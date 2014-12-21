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

#include "lyos/type.h"
#include "sys/types.h"
#include "stdio.h"
#include "unistd.h"
#include "assert.h"
#include <errno.h>
#include "lyos/const.h"
#include "string.h"
#include "lyos/proc.h"
#include "lyos/global.h"
#include "lyos/proto.h"
#include "signal.h"
#include "page.h"
#include "arch_const.h"
#include "arch_proto.h"
#ifdef CONFIG_SMP
#include "arch_smp.h"
#endif
#include "lyos/cpulocals.h"
#include <lyos/interrupt.h>

PRIVATE irq_hook_t * irq_handlers[NR_IRQ] = {0};

PUBLIC void init_irq()
{
    int i;
    for (i = 0; i < NR_IRQ_HOOKS; i++) {
        irq_hooks[i].proc_ep = NO_TASK;
    }
}

PUBLIC void put_irq_handler(int irq, irq_hook_t * hook, irq_handler_t handler)
{
    if (irq < 0 || irq >= NR_IRQ) panic("invalid irq %d", irq);
     
    irq_hook_t ** line = &irq_handlers[irq];

    int used_ids = 0;
    while (*line != NULL) {
        if (hook == *line) return;
        used_ids |= (*line)->id;
        line = &(*line)->next;
    }

    int id;
    for (id = 1; id != 0; id <<= 1) if ((used_ids & id) == 0) break;

    if (id == 0) panic("too many handlers for irq %d", irq);

    hook->next = NULL;
    hook->handler = handler;
    hook->id = id;
    hook->irq = irq;
    *line = hook;

    hwint_unmask(irq);
}

PUBLIC void irq_handle(int irq)
{
    irq_hook_t * hook = irq_handlers[irq];

    hwint_mask(irq);

    while (hook != NULL) {
        if ((*hook->handler)(hook)) /* reenable int */;

        hook = hook->next;
    }

    hwint_unmask(irq);

    hwint_ack(irq);
}

PUBLIC void enable_irq(irq_hook_t * hook)
{
    hwint_unmask(hook->irq);
}

PUBLIC int disable_irq(irq_hook_t * hook)
{
    hwint_mask(hook->irq);
    return 1;
}