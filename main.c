/*
 * main.c — RTOS 演示：双任务闪烁 LED
 *
 * 目标板: STM32F103C8T6 (Blue Pill)
 *   - PC13 = 板载 LED（低电平亮）
 *   - 系统时钟: HSI 8MHz
 *
 * 两个任务交替闪烁 LED，频率不同：
 *   TaskA: 快速闪烁（100ms 周期）— 证明任务在跑
 *   TaskB: 慢速闪烁（500ms 周期）— 证明轮转调度在工作
 *
 * 验证方法：LED 不会以固定频率闪烁，而是快慢交替 — 说明两个任务都在执行。
 */

#include "rtos.h"
#include "stm32f103.h"

/* ===== 任务栈 ========================================================= */
/*
 * 每个任务需要独立的栈空间。
 * 栈大小估算：16 个寄存器字（异常帧 8 + callee 8） + 局部变量开销
 * 128 字（512 字节）对闪烁 LED 任务绰绰有余。
 */
#define STACK_SIZE 128
static uint32_t task_a_stack[STACK_SIZE];
static uint32_t task_b_stack[STACK_SIZE];

/* ===== GPIO 初始化 ==================================================== */
static void gpio_init(void) {
  /* 使能 GPIOC 时钟 */
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

  /*
   * PC13 配置为推挽输出（50MHz）
   * CRH 控制 PC8~PC15，低 4 位对应 PC8，PC13 在 bit[23:20]。
   * MODE13[1:0] = 11b (50MHz output)
   * CNF13[1:0]  = 00b (通用推挽)
   */
  GPIOC->CRH &= ~(0xFUL << 20); /* 清掉 PC13 配置 */
  GPIOC->CRH |= (0x3UL << 20);  /* MODE=11, CNF=00 */
}

/* ===== 任务函数 ======================================================= */
/*
 * RTOS 任务必须是无限循环。
 * 任务函数本身不返回——返回只发生在上下文切换时通过异常返回机制跳转。
 */
static void task_a(void) {
  while (1) {
    GPIOC->BSRR = (1UL << 13); /* PC13 复位 → LED 亮 */
    for (volatile int i = 0; i < 200000; i++)
      ;
    GPIOC->BRR = (1UL << 13); /* PC13 置位 → LED 灭 */
    for (volatile int i = 0; i < 200000; i++)
      ;
  }
}

static void task_b(void) {
  while (1) {
    GPIOC->BSRR = (1UL << 13); /* LED 亮 */
    for (volatile int i = 0; i < 500000; i++)
      ;
    GPIOC->BRR = (1UL << 13); /* LED 灭 */
    for (volatile int i = 0; i < 500000; i++)
      ;
  }
}

/* ===== 入口 =========================================================== */
int main(void) {
  gpio_init();
  rtos_init();

  /*
   * 创建两个任务。
   * 传入的栈指针是栈顶（高地址），栈向下增长。
   * task_a_stack 数组名 = &task_a_stack[0] = 栈底（低地址），
   * 所以栈顶 = 数组名 + STACK_SIZE。
   */
  task_create(task_a_stack + STACK_SIZE, task_a);
  task_create(task_b_stack + STACK_SIZE, task_b);

  /*
   * 启动 SysTick：
   * HSI = 8MHz，reload = 8000000 / 1000 = 8000，
   * 每 1ms 触发一次中断 → 每 1ms 调用 rtos_schedule()。
   */
  rtos_systick_init(8000000);

  /*
   * SysTick 中断触发后进入 SysTick_Handler → rtos_schedule()，
   *   首次：task_start_first() → 启动 TaskA
   *   后续：task_context_switch() → TaskA ↔ TaskB 轮转
   *
   * 此处不应到达。
   */
  while (1)
    ;
}
