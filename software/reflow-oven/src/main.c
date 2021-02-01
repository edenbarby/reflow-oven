#include <stdint.h>
#include "alive.h"
#include "comms.h"
#include "controller.h"
#include "display.h"
#include "system.h"
#include "util.h"

int main(void)
{
    struct reflow_step steps[REFLOW_PROFILE_STEPS_MAX];
    struct reflow_profile profile;
    struct reflow_context context;

    profile.onset_us = 0;
    profile.len = 0;
    profile.steps = steps;

    context.profile = profile;
    context.state = REFLOW_STATE_WAIT;
    context.temp_cpu = -1.0;
    context.temp_oven = -1.0;
    context.temp_ref = -1.0;
    context.errors = 0x00;

    system_clock_init_pll_hse_72();
    system_time_init();

    while (1)
    {
        alive_task();
        comms_task(&context);
        controller_task(&context);
        display_task(&context);
    }
}