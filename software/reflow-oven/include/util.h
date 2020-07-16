#ifndef UTIL_H
#define UTIL_H

// Timer 16 Channel 1
#define BUZZER_PORT GPIOA
#define BUZZER_PIN LL_GPIO_PIN_6 // AF1

#define FTDI_PORT GPIOA
#define FTDI_RTS LL_GPIO_PIN_0 // AF7
#define FTDI_CTS LL_GPIO_PIN_1 // AF7
#define FTDI_RX LL_GPIO_PIN_2  // AF7
#define FTDI_TX LL_GPIO_PIN_3  // AF7

#define LCD_PORT GPIOB
#define LCD_CS_PIN LL_GPIO_PIN_6
#define LCD_RST_PIN LL_GPIO_PIN_7
#define LCD_MODE_PIN LL_GPIO_PIN_8
#define LCD_BACKLIGHT_PIN LL_GPIO_PIN_9

#define RELAY_PORT GPIOC
#define RELAY_1_PIN LL_GPIO_PIN_0
#define RELAY_2_PIN LL_GPIO_PIN_1
#define RELAY_3_PIN LL_GPIO_PIN_2

#define SD_PORT GPIOC
#define SD_CS_PIN LL_GPIO_PIN_6

// Timer 2 Channel 1
#define SERVO_PORT GPIOA
#define SERVO_PIN LL_GPIO_PIN_5 // AF1

#define MAX31855_PORT GPIOB
#define MAX31855_CS_PIN LL_GPIO_PIN_12

uint32_t system_hclk;
uint32_t system_pclk1;
uint32_t system_pclk2;

#endif // UTIL_H