# Makefile for my-freertos on STM32F103C8T6
#
# 目标文件:
#   firmware.elf  — ELF（含调试信息，用于 GDB / OpenOCD）
#   firmware.bin  — 原始二进制（用于串口/DFU烧录）
#
# 常用命令:
#   make            编译
#   make flash      通过 OpenOCD 烧录（需 ST-Link）
#   make clean      清理产物
#   make size       查看各段大小

# ===== 工具链 ============================================================
CROSS  = arm-none-eabi-
CC     = $(CROSS)gcc
OBJCPY = $(CROSS)objcopy
SIZE   = $(CROSS)size

# ===== 文件列表 ==========================================================
C_SRCS   = rtos.c main.c
ASM_SRCS = startup_stm32.s rtos_switch.s
OBJS     = $(C_SRCS:.c=.o) $(ASM_SRCS:.s=.o)

TARGET = firmware

# ===== 编译选项 ==========================================================
# CPU
CPU_FLAGS = -mcpu=cortex-m3 -mthumb

# C 编译
CFLAGS  = $(CPU_FLAGS)
CFLAGS += -O0 -g3                       # 无优化 + 完整调试信息
CFLAGS += -Wall -Wextra                 # 严格警告
CFLAGS += -I.                           # 当前目录加入 include path
CFLAGS += -ffunction-sections           # 未用函数可被链接器丢弃
CFLAGS += -fdata-sections
CFLAGS += --specs=nano.specs            # newlib-nano（更小的库）

# 汇编（.s 文件直接用 gcc 编译，gcc 会调用 as）
ASFLAGS  = $(CPU_FLAGS)
ASFLAGS += -c                            # 只编译不链接

# 链接
LDFLAGS  = $(CPU_FLAGS)
LDFLAGS += -T stm32f103.ld
LDFLAGS += -nostartfiles                 # 用自己的 startup
LDFLAGS += --specs=nosys.specs           # 无系统调用
LDFLAGS += --specs=nano.specs
LDFLAGS += -Wl,--gc-sections             # 丢弃未引用的 section

# ===== 编译规则 ==========================================================
.PHONY: all clean flash size

all: $(TARGET).elf $(TARGET).bin
	@$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	$(OBJCPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(ASFLAGS) -o $@ $<

clean:
	rm -f *.o $(TARGET).elf $(TARGET).bin

size: $(TARGET).elf
	$(SIZE) $<

# ===== 烧录 ==============================================================
# 默认使用 OpenOCD + ST-Link
flash: $(TARGET).bin
	openocd -f interface/stlink.cfg \
	        -f target/stm32f1x.cfg \
	        -c "program $(TARGET).bin 0x08000000 verify reset exit"
