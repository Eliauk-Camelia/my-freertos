#include "rtos.h"
#include "rtos_config.h"
#include "stddef.h"

LIST_HEAD(ready_task_head);

static struct task_struct task_pool[MAX_TASK_NUM];

static uint8_t task_count = 0;

void rtos_init(void)
{
    task_count = 0;
    INIT_LIST_HEAD(&ready_task_head);
}

/*创建任务*/
int task_create(uint32_t *stack_top,void (*func)(void))
{
    if(task_count >= MAX_TASK_NUM)
    {
        return -1;
    }

    struct task_struct *tsk = &task_pool[task_count];
    tsk->stack_top = stack_top;
    tsk->task_func =func;
    INIT_LIST_HEAD(&tsk->list);

    list_add_tail(&tsk->list,&ready_task_head);
    task_count ++;

    return 0;
}

/*遍历就绪链表，返回第一个就绪任务*/
struct task_struct *get_next_ready_task(void)
{
    struct list_head *pos;
    list_for_each(pos,&ready_task_head)
    {
        return container_of(pos,struct task_struct,list);
    }
    return NULL;
}

void rtos_schedule(void)
{
    struct task_struct *next_tsk = get_next_ready_task();
    if(!next_tsk)
    {
        while(1);
    }

    next_tsk->task_func();
}