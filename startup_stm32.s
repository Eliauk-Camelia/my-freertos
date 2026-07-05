    .syntax unified
    .arch   armv7-m
    .cpu    cortex-m3

/* ===================================================================
 * 向量表
 *
 * Cortex-M3 上电后：
 *   1. 从 0x00000000 读 MSP 初始值（或从 0x08000000，取决于 BOOT 引脚）
 *   2. 从 0x00000004 读 Reset_Handler 地址 → 跳转
 *
 * 向量表必须按 ARM 规定的顺序排列，不可调换。
 * =================================================================== */
    .section .isr_vector, "a"
    .global g_pfnVectors

g_pfnVectors:
    .word _estack                /* 0x00: 主栈指针初始值 */
    .word Reset_Handler          /* 0x04: Reset */
    .word NMI_Handler            /* 0x08: NMI */
    .word HardFault_Handler      /* 0x0C: HardFault */
    .word MemManage_Handler      /* 0x10: MemManage */
    .word BusFault_Handler       /* 0x14: BusFault */
    .word UsageFault_Handler     /* 0x18: UsageFault */
    .word 0                      /* 0x1C: Reserved */
    .word 0                      /* 0x20: Reserved */
    .word 0                      /* 0x24: Reserved */
    .word 0                      /* 0x28: Reserved */
    .word SVC_Handler            /* 0x2C: SVCall */
    .word DebugMon_Handler       /* 0x30: Debug Monitor */
    .word 0                      /* 0x34: Reserved */
    .word PendSV_Handler         /* 0x38: PendSV */
    .word SysTick_Handler        /* 0x3C: SysTick */
    /* 外设中断从 0x40 开始，此处省略（未用到） */

/* ===================================================================
 * Reset_Handler — 上电后的入口
 *
 * 按顺序完成：
 *   1. 把 .data 段从 FLASH 拷贝到 RAM（已初始化的全局变量）
 *   2. 把 .bss 段清零（未初始化的全局变量）
 *   3. 调用 main()
 *
 * 注意：进入 main 前不使能任何中断，由 RTOS 自行初始化。
 * =================================================================== */
    .text
    .thumb_func
    .global Reset_Handler
Reset_Handler:
    /* 拷贝 .data: FLASH → RAM */
    ldr r0, =_sdata      /* 目标地址（RAM） */
    ldr r1, =_edata      /* 结束地址 */
    ldr r2, =_etext      /* 源地址（FLASH 中的加载地址） */
    b   2f
1:  ldr r3, [r2], #4
    str r3, [r0], #4
2:  cmp r0, r1
    bne 1b

    /* 清零 .bss */
    ldr r0, =_sbss
    ldr r1, =_ebss
    mov r2, #0
    b   2f
1:  str r2, [r0], #4
2:  cmp r0, r1
    bne 1b

    /* 跳转到 main —— 永不返回 */
    bl  main
    b   .                   /* 安全兜底 */

/* ===================================================================
 * 默认中断处理函数（弱符号）
 *
 * .weak 的含义：
 *   如果用户在其他 .c 文件中定义了同名函数（如 SysTick_Handler），
 *   链接器使用用户的版本；否则使用此处的默认版本（死循环）。
 *   这样 rtos.c 中的 SysTick_Handler() 会自动覆盖此处的弱定义。
 * =================================================================== */
    .weak NMI_Handler
    .weak HardFault_Handler
    .weak MemManage_Handler
    .weak BusFault_Handler
    .weak UsageFault_Handler
    .weak SVC_Handler
    .weak DebugMon_Handler
    .weak PendSV_Handler
    .weak SysTick_Handler

NMI_Handler:
HardFault_Handler:
MemManage_Handler:
BusFault_Handler:
UsageFault_Handler:
SVC_Handler:
DebugMon_Handler:
PendSV_Handler:
SysTick_Handler:
    b .                         /* 死循环 — 如果触发说明未处理 */
