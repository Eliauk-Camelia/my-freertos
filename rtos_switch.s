    .syntax unified
    .arch armv7-m

/* 任务上下文保存，切换到新任务栈 */
    .global task_context_switch
task_context_switch:
    /* 第一步：保存当前任务 R4~R11 到自身栈 */
    mrs r0, psp
    stmdb r0!, {r4-r11}
    /* 把更新后的栈顶写回当前task_struct->stack_top */
    ldr r1, =current_task
    ldr r2, [r1]
    str r0, [r2]

    /* 第二步：取出下一个就绪任务 */
    bl get_next_ready_task
    ldr r1, =current_task
    str r0, [r1]

    /* 第三步：加载新任务栈，恢复 R4~R11 */
    ldr r0, [r0]
    ldmia r0!, {r4-r11}
    msr psp, r0
    bx lr
