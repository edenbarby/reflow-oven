#ifndef STATUS_LED_H
#define STATUS_LED_H

#define STATUS_LED_RED LL_GPIO_PIN_13   // LED_1
#define STATUS_LED_GREEN LL_GPIO_PIN_14 // LED_2
#define STATUS_LED_BLUE LL_GPIO_PIN_15  // LED_3

void status_led_set(uint32_t led);
void status_led_reset(uint32_t led);
void status_led_toggle(uint32_t led);

#endif // STATUS_LED_H