#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_usart.h"
#include "usart2.h"
#include "util.h"

// #define FTDI_PORT GPIOA
// #define FTDI_RTS  LL_GPIO_PIN_0
// #define FTDI_CTS  LL_GPIO_PIN_1
// #define FTDI_RX   LL_GPIO_PIN_2
// #define FTDI_TX   LL_GPIO_PIN_3

void usart2_init(void) {
    LL_GPIO_InitTypeDef  init_struct_gpio;
    LL_USART_InitTypeDef init_struct_usart;
    LL_USART_ClockInitTypeDef init_struct_usart_clock;

    // Init GPIO.
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    init_struct_gpio.Alternate  = LL_GPIO_AF_7;
    init_struct_gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    init_struct_gpio.Pin        = FTDI_RX | FTDI_TX;
    init_struct_gpio.Pull       = LL_GPIO_PULL_UP;
    init_struct_gpio.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(FTDI_PORT, &init_struct_gpio);

    // Init USART.
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_USART_StructInit(&init_struct_usart);
    LL_USART_Init(USART2, &init_struct_usart);
    LL_USART_SetBaudRate(USART2, system_pclk1, LL_USART_OVERSAMPLING_16, 115200UL);
    LL_USART_DisableOverrunDetect(USART2);
    LL_USART_ClockStructInit(&init_struct_usart_clock);
    LL_USART_ClockInit(USART2, &init_struct_usart_clock);
    LL_USART_Enable(USART2);
    LL_USART_EnableDirectionTx(USART2);
    LL_USART_EnableDirectionRx(USART2);
}

void usart2_putc(uint8_t data) {
    // LL_USART_EnableDirectionTx(USART2);
    LL_USART_TransmitData8(USART2, data);
    while(!LL_USART_IsActiveFlag_TC(USART2));
    // LL_USART_DisableDirectionTx(USART2);
}

uint8_t usart2_getc(uint8_t * data) {
    uint8_t n;
    n = 0;
    if(LL_USART_IsActiveFlag_RXNE(USART2)) {
        *data = LL_USART_ReceiveData8(USART2);
        n = 1;
    }
    return n;
    // LL_USART_EnableDirectionRx(USART2);
    // while(LL_USART_IsActiveFlag_RXNE(SPI2));
    // *data = LL_USART_ReceiveData8(USART2);
    // LL_USART_DisableDirectionRx(USART2);
}