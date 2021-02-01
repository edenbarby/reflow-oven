#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"

#include "alive.h"
#include "status_led.h"
#include "system.h"

enum alive_state
{
    ALIVE_STATE_INIT = 0,
    ALIVE_STATE_RUN = 1
};

static uint64_t alive_period;
static uint64_t alive_next;
static enum alive_state state = ALIVE_STATE_INIT;

static void alive_init(void)
{
    alive_period = system_usec2tick(500000); // 0.5s
    alive_next = system_time_get_tick() + alive_period;

    status_led_init();

    state = ALIVE_STATE_RUN;
}

static void alive_run(void)
{
    uint64_t tick_current = system_time_get_tick();
    if (alive_next < tick_current)
    {
        alive_next = tick_current + alive_period;
        status_led_toggle(STATUS_LED_GREEN);
    }
}

void alive_task(void)
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
}
