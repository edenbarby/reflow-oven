#include <stdint.h>
#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_usart.h"
#include "ring_buffer.h"
#include "system.h"
#include "usart2.h"
#include "util.h"

// #define FTDI_PORT GPIOA
// #define FTDI_RTS  LL_GPIO_PIN_0
// #define FTDI_CTS  LL_GPIO_PIN_1
// #define FTDI_RX   LL_GPIO_PIN_2
// #define FTDI_TX   LL_GPIO_PIN_3

static uint32_t overrun_errors = 0;
static uint32_t noise_errors = 0;
static uint32_t framing_errors = 0;

static uint32_t (*usart2_tx_pop)(uint8_t *);
static void (*usart2_rx_push)(uint8_t);

/*
pop A function that puts a single byte of data to be transmitted in positional argument 1 and returns a True value if there was data to pop, otherwise a False value.
push Take a single byte to store.
*/
void usart2_init(uint32_t (*tx_pop)(uint8_t *), void (*rx_push)(uint8_t))
{
    LL_GPIO_InitTypeDef init_struct_gpio;
    LL_USART_InitTypeDef init_struct_usart;
    LL_USART_ClockInitTypeDef init_struct_usart_clock;

    usart2_tx_pop = tx_pop;
    usart2_rx_push = rx_push;

    // Init GPIO.
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_StructInit(&init_struct_gpio);
    init_struct_gpio.Alternate = LL_GPIO_AF_7;
    init_struct_gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    init_struct_gpio.Pin = FTDI_RX | FTDI_TX;
    init_struct_gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(FTDI_PORT, &init_struct_gpio);

    // Init USART.
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_USART_StructInit(&init_struct_usart);
    LL_USART_Init(USART2, &init_struct_usart);
    LL_USART_SetBaudRate(USART2, system_clock_get_pclk1(), LL_USART_OVERSAMPLING_16, 115200UL);
    LL_USART_ClockStructInit(&init_struct_usart_clock);
    LL_USART_ClockInit(USART2, &init_struct_usart_clock);

    // Turn on receive buffer not empty and usart error interrupts. The
    // transmit buffer empty interrupt is only truned on when data is ready
    // to be sent.
    LL_USART_EnableIT_RXNE(USART2);
    LL_USART_EnableIT_ERROR(USART2);

    NVIC_EnableIRQ(USART2_IRQn);

    LL_USART_Enable(USART2);
    LL_USART_EnableDirectionTx(USART2);
    LL_USART_EnableDirectionRx(USART2);
}

void usart2_tx_start(void)
{
    LL_USART_EnableIT_TXE(USART2);
}

void usart2_clear_overrun(void)
{
    overrun_errors = 0;
}

void usart2_clear_noise(void)
{
    noise_errors = 0;
}

void usart2_clear_frame(void)
{
    framing_errors = 0;
}

uint32_t usart2_get_overrun(void)
{
    return overrun_errors;
}

uint32_t usart2_get_noise(void)
{
    return noise_errors;
}

uint32_t usart2_get_frame(void)
{
    return framing_errors;
}

void USART2_IRQHandler(void)
{
    uint8_t n, data;

    if (LL_USART_IsActiveFlag_TXE(USART2))
    {
        n = usart2_tx_pop(&data);
        if (n)
        {
            LL_USART_TransmitData8(USART2, data);
        }
        else
        {
            LL_USART_DisableIT_TXE(USART2);
        }
    }
    if (LL_USART_IsActiveFlag_RXNE(USART2))
    {
        usart2_rx_push(LL_USART_ReceiveData8(USART2));
    }
    if (LL_USART_IsActiveFlag_ORE(USART2))
    {
        overrun_errors++;
        LL_USART_ClearFlag_ORE(USART2);
    }
    if (LL_USART_IsActiveFlag_NE(USART2))
    {
        noise_errors++;
        LL_USART_ClearFlag_NE(USART2);
    }
    if (LL_USART_IsActiveFlag_FE(USART2))
    {
        framing_errors++;
        LL_USART_ClearFlag_FE(USART2);
    }
}