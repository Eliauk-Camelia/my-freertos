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