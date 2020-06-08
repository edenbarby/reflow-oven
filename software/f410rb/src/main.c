// /dev/serial/by-id/usb-STMicroelectronics_STM32_STLink_0676FF535155878281043432-if02


#include <stdint.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_spi.h"

#include "lcd.h"
#include "serial.h"


void init_sysclk(void);


int main(void) {
    LL_GPIO_InitTypeDef init_struct_gpio;
    LL_SPI_InitTypeDef init_struct_spi;

    init_sysclk();

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    init_struct_gpio.Alternate  = LL_GPIO_AF_5;
    init_struct_gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    init_struct_gpio.Pin        = LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    init_struct_gpio.Pull       = LL_GPIO_PULL_NO;
    init_struct_gpio.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    LL_GPIO_Init(GPIOB, &init_struct_gpio);

    init_struct_gpio.Alternate  = LL_GPIO_AF_0;
    init_struct_gpio.Mode       = LL_GPIO_MODE_OUTPUT;
    init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    init_struct_gpio.Pin        = LL_GPIO_PIN_12;
    init_struct_gpio.Pull       = LL_GPIO_PULL_NO;
    init_struct_gpio.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(GPIOB, &init_struct_gpio);
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);

    init_struct_spi.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV256;
    init_struct_spi.BitOrder          = LL_SPI_MSB_FIRST;
    init_struct_spi.ClockPhase        = LL_SPI_PHASE_1EDGE;
    init_struct_spi.ClockPolarity     = LL_SPI_POLARITY_LOW;
    init_struct_spi.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
    init_struct_spi.CRCPoly           = 15;
    init_struct_spi.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
    init_struct_spi.Mode              = LL_SPI_MODE_MASTER;
    init_struct_spi.NSS               = LL_SPI_NSS_SOFT;
    init_struct_spi.TransferDirection = LL_SPI_FULL_DUPLEX;
    LL_SPI_Init(SPI2, &init_struct_spi);
    LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_Enable(SPI2);
    // uint32_t data;

    LL_mDelay(20);

    while(1) {
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12);
        // LL_mDelay(1);
        LL_SPI_Enable(SPI2);
        LL_mDelay(1);
        LL_SPI_TransmitData8(SPI2, 0xDE);
        while(!LL_SPI_IsActiveFlag_TXE(SPI2));
        LL_SPI_TransmitData8(SPI2, 0xAD);
        while(!LL_SPI_IsActiveFlag_TXE(SPI2));
        LL_SPI_TransmitData8(SPI2, 0xBE);
        while(!LL_SPI_IsActiveFlag_TXE(SPI2));
        LL_SPI_TransmitData8(SPI2, 0xEF);
        // while(!LL_SPI_IsActiveFlag_TXE(SPI2));
        // LL_SPI_TransmitData16(SPI2, 0xDEAD);

        // LL_SPI_TransmitData16(SPI2, 0xBEEF);
        // while(!LL_SPI_IsActiveFlag_RXNE(SPI2));
        // data = (uint32_t)(LL_SPI_ReceiveData16(SPI2)) << 16;
        // while(!LL_SPI_IsActiveFlag_RXNE(SPI2));
        // while(!LL_SPI_IsActiveFlag_TXE(SPI2));
        while(LL_SPI_IsActiveFlag_BSY(SPI2));
        LL_mDelay(1);
        LL_SPI_Disable(SPI2);
        // data = (uint32_t)(LL_SPI_ReceiveData16(SPI2)) << 16;
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);
        LL_mDelay(1);


        // while(!LL_SPI_IsActiveFlag_RXNE(SPI2));
        // data = (uint32_t)(LL_SPI_ReceiveData16(SPI2)) << 16;
        // while(!LL_SPI_IsActiveFlag_BSY(SPI2));
        // LL_SPI_Disable(SPI2);
        // while(LL_SPI_IsActiveFlag_BSY(SPI2));
        // data |= (uint32_t)(LL_SPI_ReceiveData16(SPI2));
        // LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);
        // LL_mDelay(20);
    }
}

// int main(void){
//     uint8_t c;
//     uint8_t buf_index = 0;
//     char buf[100];
//
//     init_sysclk();
//     lcd_init();
//     serial_init();
//
//     for(EVER) {
//         if(LL_USART_IsActiveFlag_RXNE(USART2)) {
//             c = USART2->DR;
//             if(c == '\n') {
//                 lcd_clear();
//                 lcd_print(buf, buf_index);
//                 lcd_update();
//                 buf_index = 0;
//             } else {
//                 buf[buf_index++] = c;
//             }
//         }
//     }
//     return 0;
// }

/* Clock configuration:
** PLL Source:        HSI
** PLLM:              4
** PLLN:              50
** PLLP:              2
** SYSCLK Source:     PLL
** SYSCLK:            100MHz
** AHB1 Prescaler:    1
** AHB1:              100MHz
** HCLK:              100MHz
** APB1 Prescaler:    2
** APB1:              50MHz
** APB2 Prescaler:    1
** APB2:              100MHz
** VDD:               3.3
** Flash Wait States: 3 (table 6 from RM0401)
*/
void init_sysclk(void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_4, 50, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_Enable();
    while(LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    LL_SetSystemCoreClock(100000000);

    LL_Init1msTick(100000000);
}
