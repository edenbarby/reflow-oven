#include <stdint.h>
#include <stdio.h>
#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_utils.h"
#include "lcd.h"
#include "max31855.h"
#include "system.h"
#include "usart2.h"


int main(void) {
    uint8_t update = 0;
    uint32_t i = 0;
    uint8_t buf[70];
    char line1[12];
    char line2[12];
    uint64_t current, last_led;
    float temp1, temp2;
    LL_GPIO_InitTypeDef init_struct_gpio;
    enum MAX31855_STATUS max31855_status;

    system_clock_init_pll_hse_72();
    system_time_init();
    usart2_init();
    lcd_init();
    max31855_init();

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_StructInit(&init_struct_gpio);
    init_struct_gpio.Mode = LL_GPIO_MODE_OUTPUT;
    init_struct_gpio.Pin  = LL_GPIO_PIN_15 | LL_GPIO_PIN_14 | LL_GPIO_PIN_13 | LL_GPIO_PIN_2 | LL_GPIO_PIN_1 | LL_GPIO_PIN_0;
    LL_GPIO_Init(GPIOC, &init_struct_gpio);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_15 | LL_GPIO_PIN_14 | LL_GPIO_PIN_13 | LL_GPIO_PIN_2 | LL_GPIO_PIN_1 | LL_GPIO_PIN_0);

    current = system_time_get_usec();
    last_led = current;

    while(1) {
        current = system_time_get_usec();

        if(current - last_led > 500000ULL) {
            last_led = current;
            LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);

            if(update) {
                max31855_status = max31855_read(&temp1, &temp2);
                if (max31855_status == MAX31855_OK) {
                    snprintf(line1, 12, "  %04.4f", temp1);
                    snprintf(line2, 12, "  %04.4f", temp2);
                } else if (max31855_status == MAX31855_ERROR_COMMS) {
                    snprintf(line1, 12, "EC%04.4f", temp1);
                    snprintf(line2, 12, "  %04.4f", temp2);
                } else if (max31855_status == MAX31855_FAULT_OPEN) {
                    snprintf(line1, 12, "FO%04.4f", temp1);
                    snprintf(line2, 12, "  %4.4f", temp2);
                } else if (max31855_status == MAX31855_FAULT_SHORT_GND) {
                    snprintf(line1, 12, "FG%04.4f", temp1);
                    snprintf(line2, 12, "  %04.4f", temp2);
                } else if (max31855_status == MAX31855_FAULT_SHORT_VCC) {
                    snprintf(line1, 12, "FV%04.4f", temp1);
                    snprintf(line2, 12, "  %04.4f", temp2);
                }
                line1[11] = '#';
                line2[11] = '$';
                lcd_clear();
                lcd_print((uint8_t *)line1, 12);
                lcd_print((uint8_t *)line2, 12);
                lcd_print(buf, i);
                lcd_update();
                update = 0;
                i = 0;
            }
        }

        if(usart2_getc(&(buf[i]))) {
            usart2_putc(buf[i]);

            if(buf[i] < ' ') {
                update = 1;
            } else {
                i++;
            }
        }
    }
}