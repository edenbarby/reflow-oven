#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

/* Peripheral Interface Definitions *******************************************/
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

/* Global Limits **************************************************************/
#define REFLOW_SAFE_MAX_TEMP_OVEN 300.0f
#define REFLOW_SAFE_MAX_TEMP_REF 60.0f // Based on maximum safe to touch temperature (according to a 3 second google search).
#define REFLOW_SAFE_MAX_TEMP_CPU 60.0f
#define REFLOW_SAFE_COOL_TEMP 60.0f
#define REFLOW_PROFILE_STEPS_MAX 20
#define REFLOW_PROFILE_TIME_MAX 3600.0f // 1 hour.
#define REFLOW_PROFILE_TEMP_MIN 0.0f
#define REFLOW_PROFILE_TEMP_MAX 260.0f

/* Global States **************************************************************/
#define REFLOW_STATE_WAIT 0
#define REFLOW_STATE_RUN 1
#define REFLOW_STATE_COOL 2

/* Global Error Flags *********************************************************/
#define ERROR_TC_COMMS 0x01
#define ERROR_TC_FAULT 0x02
#define ERROR_TC_OPEN 0x03
#define ERROR_TC_SHORT_GND 0x04
#define ERROR_TC_SHORT_VCC 0x05

/* Global Data Types **********************************************************/
union byte_float_converter
{
    uint8_t b[4];
    float f;
};

struct reflow_step
{
    float time;
    float temp;
};

struct reflow_profile
{
    uint64_t onset_us;
    uint8_t len;
    struct reflow_step *steps;
};

struct reflow_context
{
    struct reflow_profile profile;
    uint8_t state;
    float temp_cpu;
    float temp_oven;
    float temp_ref;
    uint16_t errors;
};

/* Global Variables ***********************************************************/
extern struct reflow_context reflow_oven_context_g;

#endif // UTIL_H