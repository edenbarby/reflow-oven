#include <stdint.h>
#include "alive.h"
#include "comms.h"
#include "display.h"
#include "max31855.h"
#include "reflow.h"
#include "system.h"
#include "util.h"

int main(void)
{
    oven_context_t context = {
        .errors = 0,
        .state = OVEN_STATE_INIT,
        .temp_oven = 0.0,
        .temp_ref = 0.0,
        .p_current = 0.0,
        .i_current = 0.0,
        .d_current = 0.0,
        .power_current = 0.0,
        .profile = {
            .ramp_rate = 2.0,
            .soak_time = 120.0,
            .soak_temp = 150.0,
            .reflow_time = 45.0,
            .reflow_temp = 217.0,
            .controller_ramp = {
                .p = 0.1,
                .i = 0.05,
                .d = 0.05,
                .i_max = 1.0
            },
            .controller_fixed = {
                .p = 0.01,
                .i = 0.005,
                .d = 0.005,
                .i_max = 1.0
            }
        }
    };

    system_clock_init_pll_hse_72();
    system_time_init();

    while (1)
    {
        alive_task();
        comms_task(&context);
        max31855_task(&context);
        reflow_task(&context);
        display_task(&context);
    }
}