#include "rtos.h"
#include "rtos_config.h"
#include <stddef.h>

/* SysTick 寄存器定义 (Cortex-M3/M4 固定地址) */
typedef struct {
  volatile uint32_t CTRL;  /* 0x00: 控制寄存器 */
  volatile uint32_t LOAD;  /* 0x04: 重装载值 */
  volatile uint32_t VAL;   /* 0x08: 当前计数值 */
  volatile uint32_t CALIB; /* 0x0C: 校准值(只读) */
} SysTick_Type;

#define SysTick_BASE 0xE000E010UL
#define SysTick ((SysTick_Type *)SysTick_BASE)

/* CTRL 寄存器位定义 */
#define SysTick_CTRL_ENABLE (1UL << 0)    /* 使能 */
#define SysTick_CTRL_TICKINT (1UL << 1)   /* 中断使能 */
#define SysTick_CTRL_CLKSOURCE (1UL << 2) /* 时钟源: 1=系统时钟, 0=外部时钟 */

LIST_HEAD(ready_task_head);

static struct task_struct task_pool[MAX_TASK_NUM];
// 全局正在运行的任务
struct task_struct *current_task = NULL;

static uint8_t task_count = 0;

void rtos_init(void) {
  task_count = 0;
  INIT_LIST_HEAD(&ready_task_head);
}

/*栈初始化*/
void task_stack_init(struct task_struct *tsk, uint32_t *stack,
                     void (*entry)(void)) {
  uint32_t *sp = stack;
  *sp-- = 0x01000000;      // xPSR
  *sp-- = (uint32_t)entry; // PC
  *sp-- = 0xFFFFFFFD;      // LR (异常返回: 使用PSP, 返回Thread模式)
  *sp-- = 0;               // R12
  *sp-- = 0;               // R3
  *sp-- = 0;               // R2
  *sp-- = 0;               // R1
  *sp-- = 0;               // R0
  // R4~R11 由task_context_switch手工保存恢复，初始化为0
  for (int i = 0; i < 8; i++)
    *sp-- = 0;
  tsk->stack_top = sp + 4; // 指向R4，ldmia从此处开始弹出
}
/*创建任务*/
int task_create(uint32_t *stack_top, void (*func)(void)) {
  if (task_count >= MAX_TASK_NUM) {
    return -1;
  }

  struct task_struct *tsk = &task_pool[task_count];
  INIT_LIST_HEAD(&tsk->list);
  task_stack_init(tsk, stack_top, func);
  list_add_tail(&tsk->list, &ready_task_head);
  task_count++;

  return 0;
}

/*遍历就绪链表，返回第一个就绪任务*/
struct task_struct *get_next_ready_task(void) {
  struct list_head *pos;
  list_for_each(pos, &ready_task_head) {
    return container_of(pos, struct task_struct, list);
  }
  return NULL;
}

extern void task_context_switch(void);
void rtos_schedule(void) {
  if (!current_task) {
    current_task = get_next_ready_task();
    if (!current_task) {
      while (1)
        ;
    }
    current_task->task_func();
  } else {
    task_context_switch();
  }
}

void rtos_systick_init(uint32_t sys_clk) {
  /* 重装载值 = 系统时钟 / tick 频率 (1KHz → 每 1ms 一次中断) */
  uint32_t reload = sys_clk / 1000;

  /* 24 位计数器，重装载值范围 1 ~ 0x00FFFFFF */
  if (reload > 0x00FFFFFFUL) {
    reload = 0x00FFFFFFUL;
  }

  SysTick->LOAD = reload;                /* 设置重装载值 */
  SysTick->VAL = 0;                      /* 清零当前值，避免第一次中断延迟 */
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE /* 使用系统时钟 */
                  | SysTick_CTRL_TICKINT /* 使能中断 */
                  | SysTick_CTRL_ENABLE; /* 启动 SysTick */
}

/*SysTick 中断服务函数，每过tick触发一次抢占调度*/
void SysTick_Handler(void) { rtos_schedule(); }