#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "status_led.h"

#define STATUS_LED_PORT GPIOC

void status_led_init(void)
{
    LL_GPIO_InitTypeDef init_struct_gpio;
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_StructInit(&init_struct_gpio);
    init_struct_gpio.Mode = LL_GPIO_MODE_OUTPUT;
    init_struct_gpio.Pin = STATUS_LED_RED | STATUS_LED_GREEN | STATUS_LED_BLUE;
    LL_GPIO_Init(STATUS_LED_PORT, &init_struct_gpio);
    LL_GPIO_ResetOutputPin(STATUS_LED_PORT, STATUS_LED_RED | STATUS_LED_GREEN | STATUS_LED_BLUE);
}

void status_led_set(uint32_t led)
{
    LL_GPIO_SetOutputPin(STATUS_LED_PORT, led);
}

void status_led_reset(uint32_t led)
{
    LL_GPIO_ResetOutputPin(STATUS_LED_PORT, led);
}

void status_led_toggle(uint32_t led)
{
    LL_GPIO_TogglePin(STATUS_LED_PORT, led);
}