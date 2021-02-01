#include "status_led.h"
#include "controller.h"
#include "util.h"

enum controller_state
{
    CONTROLLER_STATE_INIT,
    CONTROLLER_STATE_RUN
};

static enum controller_state state = CONTROLLER_STATE_INIT;

void controller_init(void)
{
    status_led_init();
    status_led_reset(STATUS_LED_BLUE);
    state = CONTROLLER_STATE_RUN;
}

void controller_run(struct reflow_context *context)
{
    switch (context->state)
    {
    case REFLOW_STATE_WAIT:
        status_led_reset(STATUS_LED_BLUE);
        break;
    case REFLOW_STATE_RUN:
        status_led_set(STATUS_LED_BLUE);
        break;
    case REFLOW_STATE_COOL:
        status_led_reset(STATUS_LED_BLUE);
        break;
    }
}

void controller_task(struct reflow_context *context)
{
    switch (state)
    {
    case CONTROLLER_STATE_INIT:
        controller_init();
        break;
    case CONTROLLER_STATE_RUN:
        controller_run(context);
        break;
    }
}