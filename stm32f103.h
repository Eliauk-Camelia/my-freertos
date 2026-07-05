#include <stdint.h>

/*
 * STM32F103 寄存器定义（仅用到的外设）
 *
 * 不依赖 CMSIS / HAL 头文件，直接定义寄存器地址。
 * 需要更多外设时追加即可。
 */

/* ===== RCC (Reset & Clock Control) ====================================== */
#define RCC_BASE 0x40021000UL

typedef struct {
  volatile uint32_t CR;       /* 0x00: 时钟控制 */
  volatile uint32_t CFGR;     /* 0x04: 时钟配置 */
  volatile uint32_t CIR;      /* 0x08: 时钟中断 */
  volatile uint32_t APB2RSTR; /* 0x0C: APB2 外设复位 */
  volatile uint32_t APB1RSTR; /* 0x10: APB1 外设复位 */
  volatile uint32_t AHBENR;   /* 0x14: AHB 外设时钟使能 */
  volatile uint32_t APB2ENR;  /* 0x18: APB2 外设时钟使能 */
  volatile uint32_t APB1ENR;  /* 0x1C: APB1 外设时钟使能 */
} RCC_Type;

#define RCC ((RCC_Type *)RCC_BASE)

/* APB2ENR 位 */
#define RCC_APB2ENR_IOPCEN (1UL << 4) /* GPIOC 时钟使能 */

/* ===== GPIOC ============================================================= */
#define GPIOC_BASE 0x40011000UL

typedef struct {
  volatile uint32_t CRL;  /* 端口配置低 */
  volatile uint32_t CRH;  /* 端口配置高 */
  volatile uint32_t IDR;  /* 输入数据 */
  volatile uint32_t ODR;  /* 输出数据 */
  volatile uint32_t BSRR; /* 位设置/复位 */
  volatile uint32_t BRR;  /* 位复位 */
  volatile uint32_t LCKR; /* 配置锁定 */
} GPIO_Type;

#define GPIOC ((GPIO_Type *)GPIOC_BASE)
