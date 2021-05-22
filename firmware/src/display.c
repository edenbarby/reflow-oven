#include "display.h"
#include "system.h"
#include "util.h"

static enum display_state {
    DISPLAY_STATE_INIT = 0,
    DISPLAY_STATE_RUN = 1
} state = DISPLAY_STATE_INIT;

void display_init(void)
{
    state = DISPLAY_STATE_RUN;
}

void display_run(oven_context_t *context)
{
    switch (context->state)
    {
    case OVEN_STATE_INIT:
        break;
    case OVEN_STATE_IDLE:
        break;
    case OVEN_STATE_SOAK_RAMP:
        break;
    case OVEN_STATE_SOAK:
        break;
    case OVEN_STATE_REFLOW_RAMP:
        break;
    case OVEN_STATE_REFLOW:
        break;
    }
}

void display_task(oven_context_t *context)
{
    static uint64_t next = 0;
    if (system_time_get_tick() > next)
    {
        switch (state)
        {
        case DISPLAY_STATE_INIT:
            display_init();
            break;
        case DISPLAY_STATE_RUN:
            display_run(context);
            break;
        }
        next += system_usec2tick(1e5); // 0.1s
    }
}