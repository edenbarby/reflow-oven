#include <stdint.h>
#include <stdio.h>
#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_utils.h"
#include "lcd.h"
#include "max31855.h"
#include "status_led.h"
#include "system.h"
#include "usart2.h"

int main(void)
{
    // uint32_t i = 0;
    uint32_t n = 0;
    uint8_t buf[20];
    uint64_t tick_current, tick_alive_next, tick_alive_period;

    system_clock_init_pll_hse_72();
    system_time_init();
    usart2_init();
    lcd_init();
    max31855_init();

    //__enable_irq();

    tick_alive_next = 0;
    tick_alive_period = system_usec2tick(500000); // 0.5s

    while (1)
    {
        tick_current = system_time_get_tick();
        if (tick_alive_next < tick_current)
        {
            tick_alive_next = tick_current + tick_alive_period;
            status_led_toggle(STATUS_LED_RED);

            n = usart2_rx_available();
            n = n > 20 ? 20 : n;
            if (n > 0)
            {
                usart2_rx_n(buf, n);
                usart2_tx_n(buf, n);
            }
        }
    }
}