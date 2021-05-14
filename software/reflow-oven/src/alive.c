#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"

#include "alive.h"
#include "status_led.h"
#include "system.h"

static enum alive_state {
    ALIVE_STATE_INIT,
    ALIVE_STATE_RUN
} state = ALIVE_STATE_INIT;

static void alive_init(void)
{
    status_led_init();
    state = ALIVE_STATE_RUN;
}

static void alive_run(void)
{
    status_led_toggle(STATUS_LED_GREEN);
}

void alive_task(void)
{
    static uint64_t next = 0;
    if (system_time_get_tick() > next)
    {
        switch (state)
        {
        case ALIVE_STATE_INIT:
            alive_init();
            break;
        case ALIVE_STATE_RUN:
            alive_run();
            break;
        }
        next += system_usec2tick(5.0e5); // 0.5s
    }
}
