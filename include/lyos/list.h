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

#ifndef _LIST_H_
#define _LIST_H_

/* linked list facility */

struct list_head {
    struct list_head *prev, *next;
};

#define LIST_INIT(name)  \
    {                    \
        &(name), &(name) \
    }
#define DEF_LIST(name) struct list_head name = LIST_INIT(name)
#define INIT_LIST_HEAD(ptr)  \
    do {                     \
        (ptr)->next = (ptr); \
        (ptr)->prev = (ptr); \
    } while (0)

#define list_entry(ptr, type, member)                     \
    ({                                                    \
        const typeof(((type*)0)->member)* __mptr = (ptr); \
        (type*)((char*)__mptr - offsetof(type, member));  \
    })
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE*)0)->MEMBER)
#endif

PRIVATE inline int list_empty(struct list_head* list);
PRIVATE inline void list_add(struct list_head* new, struct list_head* head);
PRIVATE inline void list_del(struct list_head* node);

#define prefetch(x) __builtin_prefetch(&x)

#define list_for_each_entry(pos, head, member)                 \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         prefetch(pos->member.next), &(pos->member) != (head); \
         pos = list_entry(pos->member.next, typeof(*pos), member))
#define list_for_each_entry_safe(pos, n, head, member)       \
    for (pos = list_first_entry(head, typeof(*pos), member), \
        n = list_next_entry(pos, member);                    \
         &pos->member != (head); pos = n, n = list_next_entry(n, member))

PRIVATE inline int list_empty(struct list_head* list)
{
    return (list->next == list);
}

PRIVATE inline void __list_add(struct list_head* new, struct list_head* pre,
                               struct list_head* next)
{
    new->prev = pre;
    new->next = next;
    pre->next = new;
    next->prev = new;
}

PRIVATE inline void list_add(struct list_head* new, struct list_head* head)
{
    __list_add(new, head, head->next);
}

PRIVATE inline void list_del(struct list_head* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->prev = node;
    node->next = node;
}

#endif
