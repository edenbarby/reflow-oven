/* https://en.wikipedia.org/wiki/Exponential_smoothing#Double_exponential_smoothing
 *
 */

#include <stdint.h>

#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"

#include "max31855.h"
#include "spi.h"
#include "system.h"
#include "util.h"

#define MAX31855_OC_POS (0UL)
#define MAX31855_OC_MSK (1UL << MAX31855_OC_POS)
#define MAX31855_SCG_POS (1UL)
#define MAX31855_SCG_MSK (1UL << MAX31855_SCG_POS)
#define MAX31855_SCV_POS (2UL)
#define MAX31855_SCV_MSK (1UL << MAX31855_SCV_POS)
#define MAX31855_REFTEMP_POS (4UL)
#define MAX31855_REFTEMP_MSK (0xFFFUL << MAX31855_REFTEMP_POS)
#define MAX31855_FAULT_POS (16UL)
#define MAX31855_FAULT_MSK (1UL << MAX31855_FAULT_POS)
#define MAX31855_TCTEMP_POS (18UL)
#define MAX31855_TCTEMP_MSK (0x3FFFUL << MAX31855_TCTEMP_POS)
#define MAX31855_RESERVED ((1UL << 17UL) | (1UL << 3UL)) // Should always be 0.

static const float SAMPLE_PERIOD = 0.2;
static const uint8_t FILTER_WARMUP = 10;
static const float FILTER_WEIGHT_A = 0.16;
static const float FITLER_WEIGHT_B = 0.16;

static enum max31855_state {
    MAX31855_STATE_INIT,
    MAX31855_STATE_WARMUP,
    MAX31855_STATE_RUN
} state = MAX31855_STATE_INIT;
static struct spi_config spi_config = {
    .bit_order = LL_SPI_MSB_FIRST,
    .clock_phase = LL_SPI_PHASE_1EDGE,
    .clock_polarity = LL_SPI_POLARITY_LOW,
    .prescaler = 0};
static float filter_temp_delta_tc = 0;
static float filter_temp_delta_ref = 0;

uint16_t max31855_read(float *temp_tc, float *temp_ref)
{
    uint8_t buffer[4] = {0};
    uint16_t errors = 0;
    uint32_t reading = 0;

    LL_GPIO_ResetOutputPin(MAX31855_PORT, MAX31855_CS_PIN);

    spi_receive(&spi_config, buffer, 4);

    LL_GPIO_SetOutputPin(MAX31855_PORT, MAX31855_CS_PIN);

    reading = ((uint32_t)(buffer[0]) << 24) | ((uint32_t)(buffer[1]) << 16) |
              ((uint32_t)(buffer[2]) << 8) | (uint32_t)(buffer[3]);

    // Check that reserved bits are 0.
    if (reading & MAX31855_RESERVED)
    {
        errors |= ERROR_TC_COM;
    }

    // Check for faults.
    if (reading & MAX31855_OC_MSK)
    {
        return ERROR_TC_OPN;
    }

    if (reading & MAX31855_SCG_MSK)
    {
        return ERROR_TC_GND;
    }

    if (reading & MAX31855_SCV_MSK)
    {
        return ERROR_TC_VCC;
    }

    if (reading & MAX31855_FAULT_MSK)
    {
        return ERROR_TC_FLT;
    }

    /*
    The following explains how you take an unaligned sign integer packed into
    an unsigned integer and cast it into an aligned signed integer while
    preserving the sign. In this example we'll start with an 8 bit signed
    integer (1 sign bit and 7 data bits) packed into an unsigned 16 bit integer.

     15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
     0 | 0 |SGN|MSB| 5 | 4 | 3 | 2 | 1 |LSB| 0 | 0 | 0 | 0 | 0 | 0 |

    First you need to shift left until the SGN bit is aligned with the most
    significant bit of the desired integer size.

     15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
    SGN|MSB| 5 | 4 | 3 | 2 | 1 |LSB| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
    
    You then need to cast this as a signed integer before shifting right until
    the LSB is aligned with the least significant bit of the desired integer
    size. Because you cast the int as a signed integer before you shifted right
    into the correct alignment, the signed bit is correctly copied right as you
    shift the integer into alignment, preserving the signedness.

     15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
    SGN|SGN|SGN|SGN|SGN|SGN|SGN|SGN|MSB| 5 | 4 | 3 | 2 | 1 |LSB| 0 |
    */
    *temp_tc = (float)((int16_t)((reading & MAX31855_TCTEMP_MSK) >> 16) >> 2) / 4.0f;
    *temp_ref = (float)((int16_t)(reading & MAX31855_REFTEMP_MSK) >> MAX31855_REFTEMP_POS) / 16.0f;

    return 0;
}

void max31855_init(void)
{
    LL_GPIO_InitTypeDef init_struct_gpio;

    spi_init(&spi_config, 5000000);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    init_struct_gpio.Alternate = LL_GPIO_AF_0;
    init_struct_gpio.Mode = LL_GPIO_MODE_OUTPUT;
    init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    init_struct_gpio.Pin = MAX31855_CS_PIN;
    init_struct_gpio.Pull = LL_GPIO_PULL_UP;
    init_struct_gpio.Speed = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(MAX31855_PORT, &init_struct_gpio);

    LL_GPIO_SetOutputPin(MAX31855_PORT, MAX31855_CS_PIN);

    state = MAX31855_STATE_WARMUP;
}

void max31855_warmup(oven_context_t *context)
{
    float temp_tc = 0;
    float temp_ref = 0;
    static uint8_t filter_warmup_count = 0;
    static float temp_delta_sum_tc = 0;
    static float temp_delta_sum_ref = 0;

    context->errors |= max31855_read(&temp_tc, &temp_ref);

    if (filter_warmup_count > 0)
    {
        temp_delta_sum_tc += temp_tc - context->temp_oven;
        temp_delta_sum_ref += temp_ref - context->temp_ref;
    }

    context->temp_oven = temp_tc;
    context->temp_ref = temp_ref;

    filter_warmup_count++;

    if (filter_warmup_count > FILTER_WARMUP)
    {
        filter_temp_delta_tc = temp_delta_sum_tc / (float)(filter_warmup_count);
        filter_temp_delta_ref = temp_delta_sum_ref / (float)(filter_warmup_count);

        state = MAX31855_STATE_RUN;
    }
}

void max31855_run(oven_context_t *context)
{
    float temp_tc = 0;
    float temp_ref = 0;
    float temp_prev_tc = 0;
    float temp_prev_ref = 0;

    context->errors |= max31855_read(&temp_tc, &temp_ref);

    temp_prev_tc = context->temp_oven;
    temp_prev_ref = context->temp_ref;

    context->temp_oven = FILTER_WEIGHT_A * temp_tc + (1 - FILTER_WEIGHT_A) * (temp_prev_tc + filter_temp_delta_tc);
    context->temp_ref = FILTER_WEIGHT_A * temp_ref + (1 - FILTER_WEIGHT_A) * (temp_prev_ref + filter_temp_delta_ref);

    filter_temp_delta_tc = FITLER_WEIGHT_B * (context->temp_oven - temp_prev_tc) + (1 - FITLER_WEIGHT_B) * filter_temp_delta_tc;
    filter_temp_delta_ref = FITLER_WEIGHT_B * (context->temp_ref - temp_prev_ref) + (1 - FITLER_WEIGHT_B) * filter_temp_delta_ref;

    if (context->temp_oven > REFLOW_SAFE_MAX_TEMP_OVEN)
    {
        context->errors |= ERROR_TP_OVN;
    }
    if (context->temp_ref > REFLOW_SAFE_MAX_TEMP_REF)
    {
        context->errors |= ERROR_TP_REF;
    }
}

void max31855_task(oven_context_t *context)
{
    static uint64_t next = 0;
    if (system_time_get_tick() > next)
    {
        switch (state)
        {
        case MAX31855_STATE_INIT:
            max31855_init();
            break;
        case MAX31855_STATE_WARMUP:
            max31855_warmup(context);
            break;
        case MAX31855_STATE_RUN:
            max31855_run(context);
            break;
        }
        next += system_usec2tick(SAMPLE_PERIOD * 1e6);
    }
}