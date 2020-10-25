#include <stdint.h>
#include "lcd.h"
#include "max31855.h"
#include "status_led.h"
#include "serial.h"
#include "system.h"

#define STATE_WAIT 0x00
#define STATE_REFLOW 0x01

struct reflow_step
{
    uint16_t temp_min;
    uint16_t temp_max;
    uint16_t rate_min;
    uint16_t rate_max;
    uint16_t hold_min;
    uint16_t hold_max;
};

static uint8_t profile_len = 0;
static uint8_t profile_step = 0;
static struct reflow_step profile_buf[10];

int main(void)
{
    uint64_t tick_current;
    uint64_t tick_alive_next = 0;
    uint64_t tick_alive_period;
    uint8_t packet[300];
    uint32_t len;
    uint8_t state = STATE_WAIT;
    int16_t temp_tc;
    int16_t temp_ref;
    enum MAX31855_STATUS max31855_status;

    system_clock_init_pll_hse_72();
    system_time_init();
    serial_init();
    lcd_init();
    max31855_init();

    tick_alive_period = system_usec2tick(500000); // 0.5s

    while (1)
    {
        tick_current = system_time_get_tick();

        if (tick_alive_next < tick_current)
        {
            tick_alive_next = tick_current + tick_alive_period;
            status_led_toggle(STATUS_LED_RED);
        }

        len = serial_receive(packet, 300);
        if (len > 0)
        {
            if (packet[0] == SERIAL_TYPE_QUERY)
            {
                if (packet[1] == SERIAL_QUERY_STATE)
                {
                    serial_transmit((uint8_t[]){SERIAL_TYPE_RESP, state}, 2);
                }
                else if (packet[1] == SERIAL_QUERY_TPOVN)
                {
                    max31855_status = max31855_read(&temp_tc, &temp_ref);
                    packet[0] = SERIAL_TYPE_RESP;
                    packet[1] = (uint8_t)(max31855_status);
                    packet[2] = (uint8_t)(((uint16_t)(temp_tc ) >> 8) & 0xFF);
                    packet[3] = (uint8_t)(((uint16_t)(temp_tc ) >> 0) & 0xFF);
                    packet[4] = (uint8_t)(((uint16_t)(temp_ref) >> 8) & 0xFF);
                    packet[5] = (uint8_t)(((uint16_t)(temp_ref) >> 0) & 0xFF);
                    serial_transmit(packet, 6);
                    
                }
                else if (packet[1] == SERIAL_QUERY_TPCPU)
                {
                    serial_transmit((uint8_t[]){SERIAL_TYPE_RESP, 0, 26}, 3);
                }
                else
                {
                    //serial_transmit((uint8_t[]){SERIAL_TYPE_NACK}, 1);
                }
                
            }
            else if (packet[0] == SERIAL_TYPE_WAIT)
            {
                state = STATE_WAIT;
            }
            else if (packet[0] == SERIAL_TYPE_REFLOW)
            {
                if (state == STATE_WAIT)
                {
                    // if ((len >= 13) && ((len - 1) % 12 == 0))
                    // {
                    //     // <-1B-><- 12B->...<- 12B->
                    //     //  0:1    1:13    12n+1:12n+13
                    //     // [type][step 1]...[step n]
                        profile_len = (len - 1) / 12;
                        profile_step = 0;
                    //     for (uint16_t i = 0; (i < 10) && (12U*i+12U < len); i++)
                    //     {
                    //         profile_buf[i].temp_min = ((uint16_t)(packet[i+1]) << 8) | (uint16_t)(packet[i+2]);
                    //         profile_buf[i].temp_max = ((uint16_t)(packet[i+3]) << 8) | (uint16_t)(packet[i+4]);
                    //         profile_buf[i].rate_min = ((uint16_t)(packet[i+5]) << 8) | (uint16_t)(packet[i+6]);
                    //         profile_buf[i].rate_max = ((uint16_t)(packet[i+7]) << 8) | (uint16_t)(packet[i+8]);
                    //         profile_buf[i].hold_min = ((uint16_t)(packet[i+9]) << 8) | (uint16_t)(packet[i+10]);
                    //         profile_buf[i].hold_max = ((uint16_t)(packet[i+11]) << 8) | (uint16_t)(packet[i+12]);
                    //     }
                    profile_buf[0].temp_min = 0;

                        state = STATE_REFLOW;
                    // }
                    // else
                    // {
                    //     //serial_transmit((uint8_t[]){SERIAL_TYPE_NACK}, 1);
                    // }
                }
                
            }
            else
            {
                //serial_transmit((uint8_t[]){SERIAL_TYPE_NACK}, 1);
            }
        }

        if (state == STATE_WAIT)
        {

        }
        else if (state == STATE_REFLOW)
        {

        }
    }
}