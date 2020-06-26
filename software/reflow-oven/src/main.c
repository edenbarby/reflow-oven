#include <stdint.h>
#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_utils.h"
#include "lcd.h"
#include "system.h"


int main(void) {
    uint64_t current, last_led;
    LL_GPIO_InitTypeDef init_struct_gpio;

    system_clock_init_pll_hse_72();
    system_time_init();
    lcd_init();

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_StructInit(&init_struct_gpio);
    init_struct_gpio.Mode = LL_GPIO_MODE_OUTPUT;
    init_struct_gpio.Pin  = LL_GPIO_PIN_15 | LL_GPIO_PIN_14 | LL_GPIO_PIN_13 | LL_GPIO_PIN_2 | LL_GPIO_PIN_1 | LL_GPIO_PIN_0;
    LL_GPIO_Init(GPIOC, &init_struct_gpio);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_15 | LL_GPIO_PIN_14 | LL_GPIO_PIN_13 | LL_GPIO_PIN_2 | LL_GPIO_PIN_1 | LL_GPIO_PIN_0);

    current = system_time_get_us();
    last_led = current;

    while(1) {
        current = system_time_get_us();

        if(current - last_led > 500000ULL) {
            last_led = current;
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);

            lcd_clear();
            lcd_print("test123", 7);
            lcd_update();
            // spi2_transceive_n(data, 3);
        }
        // __HAL_SPI_ENABLE(&spi_handle);
        // HAL_SPI_TransmitReceive(&spi_handle, data, data, 3, 1000);
        // __HAL_SPI_DISABLE(&spi_handle);
    }
}