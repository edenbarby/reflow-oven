#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "max31855.h"
#include "spi.h"
#include "system.h"
#include "util.h"

#define MAX31855_OC_POS (0UL)
#define MAX31855_OC_MSK (0x1UL << MAX31855_OC_POS)
#define MAX31855_SCG_POS (1UL)
#define MAX31855_SCG_MSK (0x1UL << MAX31855_SCG_POS)
#define MAX31855_SCV_POS (2UL)
#define MAX31855_SCV_MSK (0x1UL << MAX31855_SCV_POS)
#define MAX31855_REFTEMP_POS (4UL)
#define MAX31855_REFTEMP_MSK (0xFFFUL << MAX31855_REFTEMP_POS)
#define MAX31855_FAULT_POS (16UL)
#define MAX31855_FAULT_MSK (0x1UL << MAX31855_FAULT_POS)
#define MAX31855_TCTEMP_POS (18UL)
#define MAX31855_TCTEMP_MSK (0x3FFFUL << MAX31855_TCTEMP_POS)
#define MAX31855_RESERVED ((0x1UL << 17UL) | (0x1UL << 3UL)) // Should always be 0.

static struct spi_config spi_config;

void max31855_init(void)
{
    LL_GPIO_InitTypeDef init_struct_gpio;

    spi_config.bit_order = LL_SPI_MSB_FIRST;
    spi_config.clock_phase = LL_SPI_PHASE_1EDGE;
    spi_config.clock_polarity = LL_SPI_POLARITY_LOW;
    spi_config.prescaler = 0;

    spi_init(&spi_config, 5000000);

    // Init GPIO.
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    init_struct_gpio.Alternate = LL_GPIO_AF_0;
    init_struct_gpio.Mode = LL_GPIO_MODE_OUTPUT;
    init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    init_struct_gpio.Pin = MAX31855_CS_PIN;
    init_struct_gpio.Pull = LL_GPIO_PULL_UP;
    init_struct_gpio.Speed = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(MAX31855_PORT, &init_struct_gpio);
    LL_GPIO_SetOutputPin(MAX31855_PORT, MAX31855_CS_PIN);
}

enum MAX31855_STATUS max31855_read(int16_t *temp_tc, int16_t *temp_ref)
{
    uint8_t buf[4];
    uint32_t reading;

    LL_GPIO_ResetOutputPin(MAX31855_PORT, MAX31855_CS_PIN);
    spi_receive(&spi_config, buf, 4);
    LL_GPIO_SetOutputPin(MAX31855_PORT, MAX31855_CS_PIN);

    reading = ((uint32_t)(buf[0]) << 24) | ((uint32_t)(buf[1]) << 16);
    reading |= ((uint32_t)(buf[2]) << 8) | (uint32_t)(buf[3]);

    /*
    The following explains how you take an unaligned sign integer packed into
    an unsigned integer and cast it into an aligned signed integer while
    preserving the sign. In this example we'll start with an 8 bit signed
    integer (1 sign bit and 7 data bits) pack into an unsigned 16 bit integer.

     15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
     0 | 0 |SGN|MSB| 5 | 4 | 3 | 2 | 1 |LSB| 0 | 0 | 0 | 0 | 0 | 0 |

    First you need to shift left until the SGN bit is aligned with the most
    significant bit of the desired integer size.

     15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
    SGN|MSB| 5 | 4 | 3 | 2 | 1 |LSB| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
    
    You then need to cast this as a signed integer before shifting right until
    the LSB is aligned with the least significant bit of the desired integer
    size. Because you cast the it as a signed integer before you shifted right
    into the correct alignment, the signed bit correctly copied right as you
    shifted the integer into alignment, preserving the signedness.

     15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
    SGN|SGN|SGN|SGN|SGN|SGN|SGN|SGN|MSB| 5 | 4 | 3 | 2 | 1 |LSB| 0 |
    */
    *temp_tc = (int16_t)((reading & MAX31855_TCTEMP_MSK) >> 16) >> 2;
    *temp_ref = (int16_t)(reading & MAX31855_REFTEMP_MSK) >> MAX31855_REFTEMP_POS;

    // Check that reserved bits are 0.
    if (reading & MAX31855_RESERVED)
    {
        return MAX31855_ERROR_COMMS;
    }

    // Check for faults.
    if (reading & MAX31855_OC_MSK)
    {
        return MAX31855_FAULT_OPEN;
    }

    if (reading & MAX31855_SCG_MSK)
    {
        return MAX31855_FAULT_SHORT_GND;
    }

    if (reading & MAX31855_SCV_MSK)
    {
        return MAX31855_FAULT_SHORT_VCC;
    }

    if (reading & MAX31855_FAULT_MSK)
    {
        return MAX31855_FAULT;
    }

    return MAX31855_OK;
}

float max31855_convert_temp_tc(int16_t temp_tc)
{
    return (float)(temp_tc) / 4.0f;
}

float max31855_convert_temp_ref(int16_t temp_ref)
{
    return (float)(temp_ref) / 16.0f;
}