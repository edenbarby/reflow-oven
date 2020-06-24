#include "stm32f3xx_ll_rcc.h"
#include "stm32f3xx_ll_system.h"

#include "system.h"


uint32_t system_hclk = 0;
uint32_t system_tick = 0;
uint32_t system_tick_factor_us = 1;


void system_clock_init_pll_hse_72(void) {
    // Make sure the system is running from the internal high speed
    // clock before configuring PLL and HSE.
    if(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {
        LL_RCC_HSI_Enable();
        while(!LL_RCC_HSI_IsReady());

        LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
        while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI);
    }

    // Disable PLL and HSE before configuring them.
    if(READ_BIT(RCC->CR, RCC_CR_PLLON) == (RCC_CR_PLLON)) {
        LL_RCC_PLL_Disable();
        while(LL_RCC_PLL_IsReady());
    }

    if(READ_BIT(RCC->CR, RCC_CR_HSEON) == (RCC_CR_HSEON)) {
        LL_RCC_HSE_Disable();
        while(LL_RCC_HSE_IsReady());
    }

    // Ensure the flash latency is set to two wait states for 48 < hclk (sysclk) <= 72 MHz.
    if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
        LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
        while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2);
    }

    // HSE OSC -> 12 MHz
    // PLLSRC -> HSE OSC / 2 (HSE Prescaler)
    // PLLCLK -> PLLSCR x 12 (PLLMUL) -> 72MHz
    // SYSCLK -> PLLCLK -> 72MHz
    // HCLK (AHB) -> SYSCLK / 1 (AHB Prescaler) -> 72MHz
    // PCLK1 (APB1) -> HCLK / 2 (APB1 Prescaler) -> 36MHz
    // PCLK2 (APB2) -> HCLK / 1 (APB2 Prescaler) -> 72MHz

    // Clock security system will ensure upon HSE failure that sysclk will fallback
    // to HSI and will subsequently shutdown HSE and PLL.
    // LL_RCC_HSE_EnableCSS(); // If the CSS is enabled, the CSS interrupt must be implimented and the CSSC must be cleared when the interrupt occurs.
    LL_RCC_HSE_Enable();
    while(!LL_RCC_HSE_IsReady());

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_2, LL_RCC_PLL_MUL_12);
    LL_RCC_PLL_Enable();
    while(!LL_RCC_PLL_IsReady());

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    system_hclk = 72000000;
}

void system_time_init(void) {
    // SysTick interrupt will be called every (SysTick_LOAD_RELOAD_Msk - 1) ticks.
    SysTick->LOAD = SysTick_LOAD_RELOAD_Msk;
    SysTick->VAL = 0;
    // SysTick clock is HCLK / 8.
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    // System time in microseconds == systick / system_tick_factor_us.
    system_tick_factor_us = system_hclk / (8UL * 1000000UL);
}

uint64_t system_time_get_us(void) {
    uint64_t ticks;

    // Pretend like SysTick is counting upwards.
    ticks = SysTick_LOAD_RELOAD_Msk - SysTick->VAL;
    ticks |= (uint64_t)(system_tick) << 24;

    return (uint64_t)(ticks / system_tick_factor_us);
}

void system_time_wait_us(uint64_t us) {
    uint64_t start = system_time_get_us();
    while((start + us) >= system_time_get_us());
}

void system_handler(void) {
    while(1);
}

void HardFault_Handler(void) {
    system_handler();
}

void MemManage_Handler(void) {
    system_handler();
}

void BusFault_Handler(void) {
    system_handler();
}

void UsageFault_Handler(void) {
    system_handler();
}

void SysTick_Handler(void) {
    system_tick++;
}

