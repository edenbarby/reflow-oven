#include "display.h"
#include "util.h"

enum display_state
{
    DISPLAY_STATE_INIT = 0,
    DISPLAY_STATE_RUN = 1
};

static enum display_state state = DISPLAY_STATE_INIT;

void display_init(void)
{
    state = DISPLAY_STATE_RUN;
}

void display_run(struct reflow_context *context)
{
    switch (context->state)
    {
    case REFLOW_STATE_WAIT:

        break;
    }
}

void display_task(struct reflow_context *context)
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
}