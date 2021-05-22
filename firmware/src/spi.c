#include <math.h>
#include "stm32f3xx.h"
#include "stm32f302x8.h"
#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_spi.h"
#include "spi.h"
#include "system.h"
#include "util.h"

// The SPI interface for the MAX31855, PCD8544 and the SD card.
#define SPI_GPIO_PORT GPIOB
#define SPI_GPIO_CLOCK_MSK LL_AHB1_GRP1_PERIPH_GPIOB
#define SPI_GPIO_AF LL_GPIO_AF_5
#define SPI_GPIO_PIN_SCK LL_GPIO_PIN_13
#define SPI_GPIO_PIN_MISO LL_GPIO_PIN_14
#define SPI_GPIO_PIN_MOSI LL_GPIO_PIN_15
#define SPI_GPIO_PIN_MSK ((LL_GPIO_PIN_13) | (LL_GPIO_PIN_14) | (LL_GPIO_PIN_15))
#define SPI_PERIPH SPI2
#define SPI_PERIPH_CLOCK_MSK LL_APB1_GRP1_PERIPH_SPI2

#define SPI_STATE_INITIALIZED (1UL << 0)

static uint32_t spi_state = 0;

uint32_t spi_config_update(struct spi_config *h);
uint8_t spi_transaction(uint8_t data);

uint32_t spi_init(struct spi_config *h, uint32_t baud_rate_max)
{
    uint32_t prescaler_min;

    if (h == NULL)
    {
        while (1)
        {
        }
    }

    prescaler_min = (uint32_t)(ceilf((float)(system_clock_get_pclk1()) / (float)(baud_rate_max)));
    if (prescaler_min <= 2)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV2;
    }
    else if (prescaler_min <= 4)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV4;
    }
    else if (prescaler_min <= 8)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV8;
    }
    else if (prescaler_min <= 16)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV16;
    }
    else if (prescaler_min <= 32)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV32;
    }
    else if (prescaler_min <= 64)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV64;
    }
    else if (prescaler_min <= 128)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV128;
    }
    else if (prescaler_min <= 256)
    {
        h->prescaler = LL_SPI_BAUDRATEPRESCALER_DIV256;
    }
    else
    {
        // SPI clock is too fast, bump up the APB1 prescaler.
        while (1)
        {
        }
    }

    spi_config_update(h);

    return 0;
}

uint32_t spi_config_update(struct spi_config *h)
{
    uint16_t cr1_cpy, cr1_mod;
    LL_GPIO_InitTypeDef init_struct_gpio;

    if (h == NULL)
    {
        while (1)
        {
        }
    }

    // First call, enable clocks, configure GPIO pins and set SPI defaults.
    if (!READ_BIT(spi_state, SPI_STATE_INITIALIZED))
    {
        // Initialize GPIO pins.
        LL_AHB1_GRP1_EnableClock(SPI_GPIO_CLOCK_MSK);
        init_struct_gpio.Alternate = SPI_GPIO_AF;
        init_struct_gpio.Mode = LL_GPIO_MODE_ALTERNATE;
        init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        init_struct_gpio.Pin = SPI_GPIO_PIN_MSK;
        init_struct_gpio.Pull = LL_GPIO_PULL_UP;
        init_struct_gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        LL_GPIO_Init(SPI_GPIO_PORT, &init_struct_gpio);

        LL_APB1_GRP1_EnableClock(SPI_PERIPH_CLOCK_MSK);
        // Enable master mode, set the NSS management to software mode
        // (SPI slave select now controlled interally by SSI pin), set
        // SSI pin high to remain is SPI master mode.
        WRITE_REG(SPI_PERIPH->CR1, SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR);
        WRITE_REG(SPI_PERIPH->CR2, SPI_CR2_FRXTH);

        SET_BIT(spi_state, SPI_STATE_INITIALIZED);
    }

    cr1_cpy = READ_REG(SPI_PERIPH->CR1);
    cr1_mod = cr1_cpy;
    MODIFY_REG(cr1_mod, SPI_CR1_LSBFIRST_Msk, h->bit_order);
    MODIFY_REG(cr1_mod, SPI_CR1_BR_Msk, h->prescaler);
    MODIFY_REG(cr1_mod, SPI_CR1_CPOL_Msk, h->clock_polarity);
    MODIFY_REG(cr1_mod, SPI_CR1_CPHA_Msk, h->clock_phase);

    // If changes need to be made stop SPI and apply changes.
    if (cr1_mod != cr1_cpy)
    {
        while (LL_SPI_IsActiveFlag_BSY(SPI_PERIPH))
        {
        }
        LL_SPI_Disable(SPI_PERIPH);
        WRITE_REG(SPI_PERIPH->CR1, cr1_mod);
    }

    return 0;
}

uint32_t spi_transceive(struct spi_config *h, const uint8_t *data_tx, uint8_t *data_rx, uint32_t n)
{
    uint32_t data_tx_index, data_rx_index;

    if ((h == NULL) || (data_tx == NULL) || (data_rx == NULL))
    {
        while (1)
        {
        }
    }

    data_tx_index = 0;
    data_rx_index = 0;

    spi_config_update(h);

    // Flush out the RX FIFO.
    while (LL_SPI_GetRxFIFOLevel(SPI2) != LL_SPI_RX_FIFO_EMPTY)
    {
        data_rx[data_rx_index] = LL_SPI_ReceiveData8(SPI2);
    }

    LL_SPI_Enable(SPI_PERIPH);

    while ((data_tx_index < n) || (data_rx_index < n))
    {
        if ((data_tx_index < n) && (LL_SPI_GetTxFIFOLevel(SPI_PERIPH) != LL_SPI_TX_FIFO_FULL))
        {
            LL_SPI_TransmitData8(SPI_PERIPH, data_tx[data_tx_index]);
            data_tx_index++;
        }
        // Use a while loop for RX to ensure it keeps up with the TX.
        while ((data_rx_index < n) && (LL_SPI_GetRxFIFOLevel(SPI_PERIPH) != LL_SPI_RX_FIFO_EMPTY))
        {
            data_rx[data_rx_index] = LL_SPI_ReceiveData8(SPI_PERIPH);
            data_rx_index++;
        }
    }

    while (LL_SPI_GetTxFIFOLevel(SPI2) != LL_SPI_TX_FIFO_EMPTY)
    {
    }

    while (LL_SPI_IsActiveFlag_BSY(SPI2))
    {
    }

    LL_SPI_Disable(SPI_PERIPH);

    return 0;
}

uint32_t spi_transmit(struct spi_config *h, const uint8_t *data_tx, uint32_t n)
{
    uint32_t data_tx_index;

    if ((h == NULL) || (data_tx == NULL))
    {
        while (1)
        {
        }
    }

    data_tx_index = 0;

    spi_config_update(h);

    LL_SPI_Enable(SPI_PERIPH);
    system_time_wait_usec(1);

    while (data_tx_index < n)
    {
        if (LL_SPI_GetTxFIFOLevel(SPI_PERIPH) != LL_SPI_TX_FIFO_FULL)
        {
            LL_SPI_TransmitData8(SPI_PERIPH, data_tx[data_tx_index]);
            data_tx_index++;
        }
    }

    while (LL_SPI_GetTxFIFOLevel(SPI2) != LL_SPI_TX_FIFO_EMPTY)
    {
    }

    while (LL_SPI_IsActiveFlag_BSY(SPI2))
    {
    }

    LL_SPI_Disable(SPI_PERIPH);

    return 0;
}

uint32_t spi_receive(struct spi_config *h, uint8_t *data_rx, uint32_t n)
{
    uint32_t data_tx_index, data_rx_index;

    if ((h == NULL) || (data_rx == NULL))
    {
        while (1)
        {
        }
    }

    data_tx_index = 0;
    data_rx_index = 0;

    spi_config_update(h);

    // Flush out the RX FIFO.
    while (LL_SPI_GetRxFIFOLevel(SPI2) != LL_SPI_RX_FIFO_EMPTY)
    {
        data_rx[data_rx_index] = LL_SPI_ReceiveData8(SPI2);
    }

    LL_SPI_Enable(SPI_PERIPH);
    while ((data_tx_index < n) || (data_rx_index < n))
    {
        if ((data_tx_index < n) && (LL_SPI_GetTxFIFOLevel(SPI_PERIPH) != LL_SPI_TX_FIFO_FULL))
        {
            LL_SPI_TransmitData8(SPI_PERIPH, 0xFF);
            data_tx_index++;
        }
        // Use a while loop for RX to ensure it keeps up with the TX.
        while ((data_rx_index < n) && (LL_SPI_GetRxFIFOLevel(SPI_PERIPH) != LL_SPI_RX_FIFO_EMPTY))
        {
            data_rx[data_rx_index] = LL_SPI_ReceiveData8(SPI_PERIPH);
            data_rx_index++;
        }
    }

    while (LL_SPI_GetTxFIFOLevel(SPI2) != LL_SPI_TX_FIFO_EMPTY)
    {
    }

    while (LL_SPI_IsActiveFlag_BSY(SPI2))
    {
    }

    LL_SPI_Disable(SPI_PERIPH);

    return 0;
}