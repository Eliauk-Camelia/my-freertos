    .syntax unified
    .arch armv7-m

/*
 * task_start_first — 启动第一个任务（无旧任务可保存）
 *
 * 与 task_context_switch 的区别：
 *   跳过了"保存当前R4~R11"这一步，因为系统刚启动
 *   ，没有"上一个任务"需要保存上下文。
 *   直接从 ready_task_head 取第一个任务，恢复其
 *   寄存器后通过异常返回进入Thread模式。
 */
    .global task_start_first
    .thumb_func
task_start_first:
    bl  get_next_ready_task         @ r0 = 首个就绪任务
    ldr r1, =current_task
    str r0, [r1]                    @ current_task = first task
    ldr r0, [r0]                    @ r0 = task->stack_top（指向R4）
    ldmia r0!, {r4-r11}             @ 恢复新任务R4~R11, r0前进到R0位置
    msr psp, r0                     @ PSP = 异常帧起点（R0的位置）
    /*
     * 设置EXC_RETURN = 0xFFFFFFFD:
     *   返回Thread模式 + 使用PSP作为栈指针
     *   bl 调用已覆盖了进入异常时硬件写入的lr,
     *   必须手动恢复正确的EXC_RETURN值。
     */
    ldr lr, =0xFFFFFFFD
    bx  lr                          @ 硬件出栈R0~R3,R12,LR,PC,xPSR → 任务开始运行

/*
 * task_context_switch — 保存当前任务上下文，切换到下一个任务
 *
 * 【设计要点 — 直接异常返回而非返回C调用链】
 *   如果走 "bx lr 回到 rtos_schedule → 返回 SysTick_Handler →
 *   pop {r4,pc}" 这条路径，SysTick_Handler 收尾的 pop 会从 MSP
 *   栈上弹出一个属于旧任务的 r4 值，覆盖新任务刚刚恢复的 r4。
 *   因此 task_context_switch 必须直接触发异常返回（ldr lr,=0xFFFFFFFD
 *   ; bx lr），完全绕过 C 调用链的收尾代码。
 *
 *   EXC_RETURN 硬编码为 0xFFFFFFFD 而非保存/恢复原始值的原因：
 *   - bl rtos_schedule 和 bl task_context_switch 已两次覆盖 lr，
 *     原始 EXC_RETURN 早被冲掉了，无处可恢复
 *   - 当前唯一调用路径来自 SysTick_Handler → 返回值恒为 0xFFFFFFFD
 *   - 将来若从 PendSV 等其他异常进入，需改用动态获取 EXC_RETURN
 */
    .global task_context_switch
    .thumb_func
task_context_switch:
    /* 第一步：保存当前任务 R4~R11 到自身栈 */
    mrs r0, psp
    stmdb r0!, {r4-r11}             @ 压入当前任务栈（PSP）
    ldr r1, =current_task
    ldr r2, [r1]
    str r0, [r2]                    @ 更新 task->stack_top

    /* 第二步：取出下一个就绪任务 */
    bl  get_next_ready_task         @ r0 = next task (会覆盖lr, 但我们不再需要lr)
    ldr r1, =current_task
    str r0, [r1]                    @ current_task = next

    /* 第三步：加载新任务栈，恢复 R4~R11 */
    ldr r0, [r0]                    @ r0 = next->stack_top（指向R4）
    ldmia r0!, {r4-r11}             @ 恢复新任务R4~R11, r0前进到R0位置
    msr psp, r0                     @ PSP = 异常帧起点

    /* 第四步：直接异常返回，不经过任何C调用链 */
    ldr lr, =0xFFFFFFFD             @ EXC_RETURN: Thread模式 + PSP
    bx  lr                          @ 硬件出栈R0~R3,R12,LR,PC,xPSR
