#ifndef __RTOS_H__
#define __RTOS_H__

#include <stdint.h>

struct list_head{
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD_INIT(name) {&(name),&(name)}
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

static inline void __list_add(struct list_head *new_node, struct list_head *prev, struct list_head *next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

static inline void list_add_tail(struct list_head *new_node,struct list_head *head)
{
    __list_add(new_node,head->prev,head);
}

static inline void list_del(struct list_head *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = nullptr;
    entry->prev = nullptr;
}

/* 核心宏：通过链表节点反向拿到宿主结构体（对应FreeRTOS偏移计算） */
#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) *__mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member));})

#define list_for_each(pos,head) for(pos = (head)->next ;pos != head;pos = pos->next)

/*任务结构体*/
struct task_struct{
    uint32_t *stack_top;
    void (*task_func)(void);
    struct list_head list;
};

#endif
