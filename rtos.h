#ifndef __RTOS_H__
#define __RTOS_H__
#include <rtos_config.h>
#include <stdint.h>

struct list_head {
  struct list_head *next;
  struct list_head *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *head) {
  head->next = head;
  head->prev = head;
}

static inline void __list_add(struct list_head *new_node,
                              struct list_head *prev, struct list_head *next) {
  next->prev = new_node;
  new_node->next = next;
  new_node->prev = prev;
  prev->next = new_node;
}

static inline void list_add_tail(struct list_head *new_node,
                                 struct list_head *head) {
  __list_add(new_node, head->prev, head);
}

/* 移除一个链表节点 */
static inline void list_del(struct list_head *entry) {
  struct list_head *prev = entry->prev;
  struct list_head *next = entry->next;
  next->prev = prev;
  prev->next = next;
}

/* 核心宏：通过链表节点反向拿到宿主结构体（对应FreeRTOS偏移计算） */
#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    const typeof(((type *)0)->member) *__mptr = (ptr);                         \
    (type *)((char *)__mptr - offsetof(type, member));                         \
  })

#define list_for_each(pos, head)                                               \
  for (pos = (head)->next; pos != head; pos = pos->next)

/*任务结构体*/
struct task_struct {
  uint32_t *stack_top; // 栈顶指针
  void (*task_func)(void);
  struct list_head list;
};
/* ===== RTOS 内核 API ==================================================== */
void rtos_init(void);
int task_create(uint32_t *stack_top, void (*func)(void));
void rtos_schedule(void);
void rtos_systick_init(uint32_t sys_clk);
struct task_struct *get_next_ready_task(void);

/* ===== 汇编实现 =========================================================== */
extern void task_start_first(void);
extern void task_context_switch(void);

#endif
