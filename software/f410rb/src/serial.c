/*
USART2
*/


#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"

#include "serial.h"
#include "queue.h"


/* UART2 interface. */
#define SERIAL_PORT   GPIOA
#define SERIAL_PIN_TX LL_GPIO_PIN_2
#define SERIAL_PIN_RX LL_GPIO_PIN_3


#define QUEUE_RX_SIZE (1<<7) //128
#define QUEUE_TX_SIZE (1<<7) //128


static uint8_t array_rx[QUEUE_RX_SIZE];
static uint8_t array_tx[QUEUE_TX_SIZE];
static struct QueueHandler queue_rx;
static struct QueueHandler queue_tx;


void serial_init(void) {
    LL_GPIO_InitTypeDef init_struct_gpio;
    LL_USART_InitTypeDef init_struct_usart;
    // LL_USART_ClockInitTypeDef init_struct_usart_clock;

    queue_init(&queue_rx, QUEUE_RX_SIZE, array_rx);
    queue_init(&queue_tx, QUEUE_TX_SIZE, array_tx);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

    LL_GPIO_StructInit(&init_struct_gpio);
    init_struct_gpio.Alternate = LL_GPIO_AF_7;
    init_struct_gpio.Mode      = LL_GPIO_MODE_ALTERNATE;
    init_struct_gpio.Pin       = SERIAL_PIN_TX | SERIAL_PIN_RX;
    init_struct_gpio.Speed     = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    LL_GPIO_Init(SERIAL_PORT, &init_struct_gpio);



    LL_USART_StructInit(&init_struct_usart);
    init_struct_usart.BaudRate            = 115200;
    // init_struct_usart.DataWidth           = LL_USART_DATAWIDTH_8B;
    // init_struct_usart.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    // init_struct_usart.OverSampling        = LL_USART_OVERSAMPLING_16;
    // init_struct_usart.Parity              = LL_USART_PARITY_NONE;
    // init_struct_usart.StopBits            = LL_USART_STOPBITS_1;
    // init_struct_usart.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    LL_USART_Init(USART2, &init_struct_usart);

    // //LL_USART_ClockStructInit(&init_struct_usart_clock);
    // USART_ClockInitStruct->ClockOutput       = LL_USART_CLOCK_DISABLE;
    // USART_ClockInitStruct->ClockPolarity     = LL_USART_POLARITY_LOW;            /* Not relevant when ClockOutput = LL_USART_CLOCK_DISABLE */
    // USART_ClockInitStruct->ClockPhase        = LL_USART_PHASE_1EDGE;             /* Not relevant when ClockOutput = LL_USART_CLOCK_DISABLE */
    // USART_ClockInitStruct->LastBitClockPulse = LL_USART_LASTCLKPULSE_NO_OUTPUT;
    // LL_USART_ClockInit(USART2, &init_struct_usart_clock);

    // LL_USART_EnableIT_RXNE(USART2);
    // LL_USART_EnableIT_TXE(USART2);

    // // Enable the interrupts.
    // NVIC_DisableIRQ(USART2_IRQn);
    // NVIC_ClearPendingIRQ(USART2_IRQn);
    // NVIC_SetPriority(USART2_IRQn, 7);
    // NVIC_EnableIRQ(USART2_IRQn);

    LL_USART_ConfigAsyncMode(USART2);
    LL_USART_Enable(USART2);
}

void serial_write(void) {
    LL_USART_TransmitData8(USART2, 'h');
    while(!LL_USART_IsActiveFlag_TXE(USART2));
    LL_USART_TransmitData8(USART2, 'i');
    while(!LL_USART_IsActiveFlag_TXE(USART2));
    LL_USART_TransmitData8(USART2, '!');
    while(!LL_USART_IsActiveFlag_TXE(USART2));
    LL_USART_TransmitData8(USART2, '\n');
    while(!LL_USART_IsActiveFlag_TXE(USART2));
}

void USART2_IRQHandler(void) {

}
