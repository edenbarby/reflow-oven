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
#define REFLOW_PROFILE_STEPS_MIN 2
#define REFLOW_PROFILE_STEPS_MAX 20
#define REFLOW_PROFILE_TIME_MAX 3600.0f // 1 hour.
#define REFLOW_PROFILE_TEMP_MIN 0.0f
#define REFLOW_PROFILE_TEMP_MAX 260.0f

/* Global Error Flags *********************************************************/
#define ERROR_TC_COM (1 << 0) // Thermocouple IC communications error.
#define ERROR_TC_FLT (1 << 1) // Thermocouple unknown fault.
#define ERROR_TC_OPN (1 << 2) // Thermocouple disconnected.
#define ERROR_TC_GND (1 << 3) // Thermocouple shorted to ground.
#define ERROR_TC_VCC (1 << 4) // Thermocouple shorted to VCC.
#define ERROR_TP_OVN (1 << 5) // Oven temperature exceeded maximum.
#define ERROR_TP_REF (1 << 6) // Thermocouple IC temperature exceeded maximum.

/* Global States **************************************************************/
#define OVEN_STATE_INIT 0
#define OVEN_STATE_IDLE 1
#define OVEN_STATE_SOAK_RAMP 2
#define OVEN_STATE_SOAK 3
#define OVEN_STATE_REFLOW_RAMP 4
#define OVEN_STATE_REFLOW 5

/* Global Data Types **********************************************************/
typedef struct
{
    float p;
    float i;
    float d;
    float i_max;
} pid_settings_t;

typedef struct
{
    float ramp_rate;
    float soak_time;
    float soak_temp;
    float reflow_time;
    float reflow_temp;
    pid_settings_t controller_ramp;
    pid_settings_t controller_fixed;
} reflow_profile_t;

typedef struct
{
    uint32_t errors;
    uint32_t state;
    float temp_oven;
    float temp_ref;
    float p_current;
    float i_current;
    float d_current;
    float power_current;
    reflow_profile_t profile;
} oven_context_t;

#endif // UTIL_H