/*
 * STM32F103C8T6 链接脚本
 *
 * FLASH: 64KB @ 0x08000000
 * RAM:   20KB @ 0x20000000
 */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* 栈顶 = RAM末尾，向下增长 */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* ============ FLASH 区域 ======================================== */

    /* 向量表必须放在最前面 */
    .isr_vector : {
        KEEP(*(.isr_vector))
    } > FLASH

    /* .text = 代码 + 常量 */
    .text : {
        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
        _etext = .;          /* .data 在 FLASH 中的加载起始地址 */
    } > FLASH

    /* ============ RAM 区域 ========================================== */

    .data : AT(_etext) {
        . = ALIGN(4);
        _sdata = .;          /* .data 在 RAM 中的起始地址 */
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _edata = .;          /* .data 在 RAM 中的结束地址 */
    } > RAM

    /* .bss = 未初始化全局变量，启动时清零 */
    .bss : {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}
