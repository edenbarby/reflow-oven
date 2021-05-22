#include <math.h>
#include "status_led.h"
#include "reflow.h"
#include "heating.h"
#include "system.h"
#include "util.h"

static uint8_t reflow_controller_reset = 1;

static const float next_temp_threshold = 5;

void reflow_init(oven_context_t *context)
{
    heating_init();
    status_led_init();
    status_led_reset(STATUS_LED_BLUE);
    context->state = OVEN_STATE_IDLE;
}

void reflow_idle(void)
{
    heating_off(HEATING_ELEMENT_ALL);
    status_led_reset(STATUS_LED_BLUE);
}

void reflow_run(oven_context_t *context)
{
    static uint8_t next_state = OVEN_STATE_IDLE;
    static uint8_t next_condition = 0; // 0 - time, 1 - temp
    static float next_time = 0;
    static float next_temp = -INFINITY;

    float time;
    static float time_prev = 0.0;
    float time_delta;
    float temp;
    static float temp_prev = 0.0;
    float error;
    static float error_prev = 0.0;
    static float error_i = 0.0;
    float error_d;
    static float p = 0;
    static float i = 0;
    static float d = 0;
    static float i_max = 0;
    float power;

    status_led_set(STATUS_LED_BLUE);

    time = (float)(system_time_get_usec()) / 1e6;
    time_delta = time - time_prev;
    temp = context->temp_oven;

    if (reflow_controller_reset == 1)
    {
        if ((context->state == OVEN_STATE_SOAK_RAMP) || (context->state == OVEN_STATE_REFLOW_RAMP))
        {
            if (context->state == OVEN_STATE_SOAK_RAMP)
            {
                next_state = OVEN_STATE_SOAK;
                next_temp = context->profile.soak_temp - next_temp_threshold;
            }
            else
            {
                next_state = OVEN_STATE_REFLOW;
                next_temp = context->profile.reflow_temp - next_temp_threshold;
            }
            next_condition = 1;
            next_time = 0;
            
            p = context->profile.controller_ramp.p;
            i = context->profile.controller_ramp.i;
            d = context->profile.controller_ramp.d;
            i_max = context->profile.controller_ramp.i_max;
        }
        else if ((context->state == OVEN_STATE_SOAK) || (context->state == OVEN_STATE_REFLOW))
        {
            if (context->state == OVEN_STATE_SOAK)
            {
                next_state = OVEN_STATE_REFLOW_RAMP;
                next_time = context->profile.soak_time + time;
            }
            else
            {
                next_state = OVEN_STATE_IDLE;
                next_time = context->profile.reflow_time + time;
            }
            next_condition = 0;
            next_temp = -INFINITY;

            p = context->profile.controller_fixed.p;
            i = context->profile.controller_fixed.i;
            d = context->profile.controller_fixed.d;
            i_max = context->profile.controller_fixed.i_max;
        }

        reflow_controller_reset = 0;
        time_prev = time;
        temp_prev = temp;
        error_prev = 0;
        error_i = 0;
    }
    else if (time_delta > 0)
    {
        switch (context->state)
        {
        case OVEN_STATE_SOAK_RAMP:
        case OVEN_STATE_REFLOW_RAMP:
            error = context->profile.ramp_rate - (temp - temp_prev) / time_delta;
            break;
        case OVEN_STATE_SOAK:
            error = context->profile.soak_temp - temp;
            break;
        case OVEN_STATE_REFLOW:
            error = context->profile.reflow_temp - temp;
            break;
        default:
            while (1)
            {
            }
        }

        error_i += time_delta * (error + error_prev) / 2;
        error_d = (error - error_prev) / time_delta;

        if ((fabsf(i) > 1.0e-10) && (fabsf(i_max) > 1.0e-10))
        {
            error_i = i * error_i > i_max ? (i_max / i) : error_i;
            error_i = i * error_i < -i_max ? -(i_max / i) : error_i;
        }

        power = p * error + i * error_i + d * error_d;

        heating_on(HEATING_ELEMENT_TOP | HEATING_ELEMENT_BOT, power);

        time_prev = time;
        temp_prev = temp;
        error_prev = error;

        context->p_current = p * error;
        context->i_current = i * error_i;
        context->d_current = d * error_d;
        context->power_current = power;
    }

    if (((next_condition == 0) && (time > next_time)) ||
        ((next_condition == 1) && (temp > next_temp)))
    {
        context->state = next_state;
    }
}

void reflow_task(oven_context_t *context)
{
    static uint8_t state_prev = OVEN_STATE_INIT;
    static uint64_t next = 0;

    if (system_time_get_tick() > next)
    {
        if (context->errors != 0)
        {
            context->state = OVEN_STATE_IDLE;
        }

        switch (context->state)
        {
        case OVEN_STATE_INIT:
            reflow_init(context);
            break;
        case OVEN_STATE_IDLE:
            reflow_idle();
            break;
        case OVEN_STATE_SOAK_RAMP:
        case OVEN_STATE_SOAK:
        case OVEN_STATE_REFLOW_RAMP:
        case OVEN_STATE_REFLOW:
            reflow_run(context);
            break;
        }

        if (state_prev != context->state)
        {
            reflow_controller_reset = 1;
            state_prev = context->state;
        }

        next += system_usec2tick(1e6); // 1s
    }
}