#include "rtos.h"
#include "rtos_config.h"
#include <stddef.h>


LIST_HEAD(ready_task_head);

static struct task_struct task_pool[MAX_TASK_NUM];
// 全局正在运行的任务
struct task_struct *current_task = NULL;

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
    INIT_LIST_HEAD(&tsk->list);
    task_stack_init(tsk,stack_top,func);
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

extern void task_context_switch(void);
void rtos_schedule(void)
{
    if(!current_task)
    {
        current_task = get_next_ready_task();
        if(!current_task)
        {
            while(1);
        }
        current_task->task_func();
    }
    else
    {
        task_context_switch();
    }
}


/*栈初始化*/
void task_stack_init(struct task_struct *tsk,uint32_t *stack,void(*entry)(void))
{
    uint32_t *sp = stack;
    *sp-- = 0x01000000; // xPSR
    *sp-- = (uint32_t)entry; // PC
    *sp-- = 0xFFFFFFFD; // LR
    // R0-R3,R12 随意填充
    for(int i=0;i<4;i++) *sp--=0;
    *sp-- = 0; // R12
    // R4~R11 切换时手动保存恢复，初始0
    for(int i=0;i<8;i++) *sp--=0;
    tsk->stack_top = sp;
}