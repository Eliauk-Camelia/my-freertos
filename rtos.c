#include "rtos.h"
#include "rtos_config.h"

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

