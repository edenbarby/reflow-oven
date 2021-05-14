#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_rcc.h"
#include "stm32f3xx_ll_tim.h"
#include "heating.h"
#include "system.h"
#include "util.h"

#define HEATING_ELEMENT_PORT RELAY_PORT

#define HEATING_PWM_FREQUENCY 5U
#define HEATING_PWM_PRESCALER 0xFFFFU

static uint32_t heating_pwm_autoreload = 0;

void heating_init(void)
{
    uint32_t clock_source;
    LL_GPIO_InitTypeDef init_struct_gpio;
    LL_TIM_InitTypeDef init_struct_tim;
    LL_TIM_OC_InitTypeDef init_struct_tim_oc;

    // Timer 1 clock source is PCLK2 x1 (or x2 if APB2 prescaler > 1)
    LL_RCC_SetTIMClockSource(LL_RCC_TIM1_CLKSOURCE_PCLK2);
    if(LL_RCC_GetAPB2Prescaler() == LL_RCC_APB2_DIV_1)
    {
        clock_source = system_clock_get_pclk2();
    }
    else
    {
        clock_source = 2 * system_clock_get_pclk2();
    }

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_StructInit(&init_struct_gpio);
    init_struct_gpio.Pin       = HEATING_ELEMENT_ALL;
    init_struct_gpio.Mode      = LL_GPIO_MODE_ALTERNATE;
    init_struct_gpio.Pull      = LL_GPIO_PULL_DOWN;
    init_struct_gpio.Alternate = LL_GPIO_AF_2;
    LL_GPIO_Init(HEATING_ELEMENT_PORT, &init_struct_gpio);

    heating_pwm_autoreload = (uint16_t)((clock_source) / (HEATING_PWM_FREQUENCY * HEATING_PWM_PRESCALER) - 1);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
    LL_TIM_EnableARRPreload(TIM1);
    LL_TIM_StructInit(&init_struct_tim);
    init_struct_tim.Prescaler  = HEATING_PWM_PRESCALER;
    init_struct_tim.Autoreload = heating_pwm_autoreload;
    LL_TIM_Init(TIM1, &init_struct_tim);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH2);
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH3);
    LL_TIM_OC_StructInit(&init_struct_tim_oc);
    init_struct_tim_oc.OCMode       = LL_TIM_OCMODE_PWM1;
    init_struct_tim_oc.OCState      = LL_TIM_OCSTATE_ENABLE;
    init_struct_tim_oc.CompareValue = 0;
    LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &init_struct_tim_oc);
    LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH2, &init_struct_tim_oc);
    LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH3, &init_struct_tim_oc);
    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
    LL_TIM_EnableAllOutputs(TIM1);
    LL_TIM_EnableCounter(TIM1);
}

void heating_on(uint32_t element, float power)
{
    uint32_t compare;

    power = power > 1.0 ? 1.0 : power;
    power = power < 0.0 ? 0.0 : power;
    compare = (uint32_t)(power * (float)(heating_pwm_autoreload));

    if(element & RELAY_1_PIN)
    {
        LL_TIM_OC_SetCompareCH1(TIM1, compare);
    }
    if(element & RELAY_2_PIN)
    {
        LL_TIM_OC_SetCompareCH2(TIM1, compare);
    }
    if(element & RELAY_3_PIN)
    {
        LL_TIM_OC_SetCompareCH3(TIM1, compare);
    }
}

void heating_off(uint32_t element)
{
    if(element & RELAY_1_PIN)
    {
        LL_TIM_OC_SetCompareCH1(TIM1, 0U);
    }
    if(element & RELAY_2_PIN)
    {
        LL_TIM_OC_SetCompareCH2(TIM1, 0U);
    }
    if(element & RELAY_3_PIN)
    {
        LL_TIM_OC_SetCompareCH3(TIM1, 0U);
    }
}