#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_usart.h"
#include "ring_buffer.h"
#include "usart2.h"
#include "util.h"

// #define FTDI_PORT GPIOA
// #define FTDI_RTS  LL_GPIO_PIN_0
// #define FTDI_CTS  LL_GPIO_PIN_1
// #define FTDI_RX   LL_GPIO_PIN_2
// #define FTDI_TX   LL_GPIO_PIN_3

// Note that the ring buffer will only utilize CAPACITY - 1.
#define TX_BUFFER_CAPACITY 300
#define RX_BUFFER_CAPACITY 300

static uint8_t tx_buffer[TX_BUFFER_CAPACITY];
static uint8_t rx_buffer[RX_BUFFER_CAPACITY];
static ring_buffer_handle tx_buffer_h = {TX_BUFFER_CAPACITY, 0, 0, tx_buffer};
static ring_buffer_handle rx_buffer_h = {RX_BUFFER_CAPACITY, 0, 0, rx_buffer};

void usart2_init(void)
{
    LL_GPIO_InitTypeDef init_struct_gpio;
    LL_USART_InitTypeDef init_struct_usart;
    LL_USART_ClockInitTypeDef init_struct_usart_clock;

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
    LL_USART_SetBaudRate(USART2, system_pclk1, LL_USART_OVERSAMPLING_16, 115200UL);
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

uint32_t usart2_tx_available(void)
{
    return ring_buffer_remaining(&tx_buffer_h);
}

uint32_t usart2_rx_available(void)
{
    return ring_buffer_used(&rx_buffer_h);
}

void usart2_tx_n(const uint8_t *data, uint32_t n)
{
    uint32_t i = 0;

    while (i < n)
    {
        i += ring_buffer_push_n(&tx_buffer_h, &(data[i]), (n - i));
        // TODO: track maximum tx buffer usage
        if (!LL_USART_IsEnabledIT_TXE(USART2))
        {
            LL_USART_EnableIT_TXE(USART2);
        }
    }
}

void usart2_rx_n(uint8_t *data, uint32_t n)
{
    uint32_t i = 0;

    while (i < n)
    {
        i += ring_buffer_pop_n(&rx_buffer_h, &(data[i]), (n - i));
    }
}

void USART2_IRQHandler(void)
{
    uint8_t n, data;

    if (LL_USART_IsActiveFlag_TXE(USART2))
    {
        n = ring_buffer_pop(&tx_buffer_h, &data);
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
        data = LL_USART_ReceiveData8(USART2);
        n = ring_buffer_push(&rx_buffer_h, data);
        // TODO: track maximum rx buffer usage
        if (n == 0)
        {
            // TODO: ring buffer overrun error
        }
    }
    if (LL_USART_IsActiveFlag_ORE(USART2))
    {
        // TODO: USART overrun error
        LL_USART_ClearFlag_ORE(USART2);
    }
    if (LL_USART_IsActiveFlag_NE(USART2))
    {
        // TODO: noise error
        LL_USART_ClearFlag_NE(USART2);
    }
    if (LL_USART_IsActiveFlag_FE(USART2))
    {
        // TODO: framing error
        LL_USART_ClearFlag_FE(USART2);
    }
}