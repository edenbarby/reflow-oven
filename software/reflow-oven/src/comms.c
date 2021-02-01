#include "comms.h"
#include "serial.h"
#include "system.h"

#define COMMS_TYPE_NACK 0
#define COMMS_TYPE_CMD_WAIT 10
#define COMMS_TYPE_CMD_RUN 11
#define COMMS_TYPE_CMD_COOL 12
#define COMMS_TYPE_QUERY_STATE 20
#define COMMS_TYPE_QUERY_TPOVN 21
#define COMMS_TYPE_QUERY_TPCPU 22

enum comms_state
{
    COMMS_STATE_INIT,
    COMMS_STATE_RUN
};

static enum comms_state state = COMMS_STATE_INIT;

uint8_t comms_reflow_length_check(uint32_t packet_len)
{
    /* There are two elements to check here; the first is to ensure that the
    ** packet contains an integer number of reflow steps plus the type header
    ** (i.e. 8n+1) and the second is to check that the number of steps is less
    ** than REFLOW_PROFILE_STEPS_MAX (n < REFLOW_PROFILE_STEPS_MAX).
    */
    return ((packet_len - 1) % 8 == 0) && (((packet_len - 1) / 8) <= REFLOW_PROFILE_STEPS_MAX);
}

uint32_t comms_reflow_unpacker(uint8_t *packed, uint32_t packed_len, struct reflow_step *unpacked)
{
    uint32_t i;
    union byte_float_converter byte2float;

    for (i = 0; (8 * (i + 1) <= packed_len) && (i < REFLOW_PROFILE_STEPS_MAX); i++)
    {
        byte2float.b[0] = packed[8 * i];
        byte2float.b[1] = packed[8 * i + 1];
        byte2float.b[2] = packed[8 * i + 2];
        byte2float.b[3] = packed[8 * i + 3];
        unpacked[i].time = byte2float.f;
        byte2float.b[0] = packed[8 * i + 4];
        byte2float.b[1] = packed[8 * i + 5];
        byte2float.b[2] = packed[8 * i + 6];
        byte2float.b[3] = packed[8 * i + 7];
        unpacked[i].temp = byte2float.f;
    }

    return i;
}

void comms_init(void)
{
    serial_init();
    state = COMMS_STATE_RUN;
}

void comms_run(struct reflow_context *context)
{
    uint32_t tx_len = 0;
    uint32_t rx_len;
    uint8_t packet_buf[SERIAL_PACKET_MAX_LEN];
    union byte_float_converter float2byte;

    rx_len = serial_receive(packet_buf, SERIAL_PACKET_MAX_LEN);

    if (rx_len > 0)
    {
        switch (packet_buf[0])
        {
        case COMMS_TYPE_CMD_WAIT:
            if (rx_len == 1)
            {
                context->state = REFLOW_STATE_WAIT;

                packet_buf[tx_len++] = COMMS_TYPE_CMD_WAIT;
                packet_buf[tx_len++] = (uint8_t)(context->errors >> 8);
                packet_buf[tx_len++] = (uint8_t)(context->errors);
            }
            break;
        case COMMS_TYPE_CMD_RUN:
            if (comms_reflow_length_check(rx_len))
            {
                context->state = REFLOW_STATE_RUN;
                context->profile.onset_us = system_time_get_usec();
                context->profile.len = comms_reflow_unpacker(&packet_buf[1], rx_len - 1, context->profile.steps);

                packet_buf[tx_len++] = COMMS_TYPE_CMD_RUN;
                packet_buf[tx_len++] = (uint8_t)(context->errors >> 8);
                packet_buf[tx_len++] = (uint8_t)(context->errors);
            }
            break;
        case COMMS_TYPE_CMD_COOL:
            if (rx_len == 1)
            {
                context->state = REFLOW_STATE_COOL;

                packet_buf[tx_len++] = COMMS_TYPE_CMD_COOL;
                packet_buf[tx_len++] = (uint8_t)(context->errors >> 8);
                packet_buf[tx_len++] = (uint8_t)(context->errors);
            }
            break;
        case COMMS_TYPE_QUERY_STATE:
            if (rx_len == 1)
            {
                packet_buf[tx_len++] = COMMS_TYPE_QUERY_STATE;
                packet_buf[tx_len++] = (uint8_t)(context->errors >> 8);
                packet_buf[tx_len++] = (uint8_t)(context->errors);
                packet_buf[tx_len++] = context->state;
            }
            break;
        case COMMS_TYPE_QUERY_TPOVN:
            if (rx_len == 1)
            {
                packet_buf[tx_len++] = COMMS_TYPE_QUERY_TPOVN;
                packet_buf[tx_len++] = (uint8_t)(context->errors >> 8);
                packet_buf[tx_len++] = (uint8_t)(context->errors);
                float2byte.f = context->temp_oven;
                packet_buf[tx_len++] = float2byte.b[0];
                packet_buf[tx_len++] = float2byte.b[1];
                packet_buf[tx_len++] = float2byte.b[2];
                packet_buf[tx_len++] = float2byte.b[3];
                float2byte.f = context->temp_ref;
                packet_buf[tx_len++] = float2byte.b[0];
                packet_buf[tx_len++] = float2byte.b[1];
                packet_buf[tx_len++] = float2byte.b[2];
                packet_buf[tx_len++] = float2byte.b[3];
            }
            break;
        case COMMS_TYPE_QUERY_TPCPU:
            if (rx_len == 1)
            {
                packet_buf[tx_len++] = COMMS_TYPE_QUERY_TPCPU;
                packet_buf[tx_len++] = (uint8_t)(context->errors >> 8);
                packet_buf[tx_len++] = (uint8_t)(context->errors);
                float2byte.f = context->temp_cpu;
                packet_buf[tx_len++] = float2byte.b[0];
                packet_buf[tx_len++] = float2byte.b[1];
                packet_buf[tx_len++] = float2byte.b[2];
                packet_buf[tx_len++] = float2byte.b[3];
            }
            break;
        }

        if (tx_len == 0)
        {
            packet_buf[tx_len++] = COMMS_TYPE_NACK;
        }

        if (serial_transmit(packet_buf, tx_len) <= tx_len)
        {
            /* Because of the COBS encoding the transmitted bytes should always
            ** be greater than the unencoded data therefore something has gone
            ** wrong transmitting this packet.
            */
            while (1)
                ;
        }
    }
}

void comms_task(struct reflow_context *context)
{
    switch (state)
    {
    case COMMS_STATE_INIT:
        comms_init();
        break;
    case COMMS_STATE_RUN:
        comms_run(context);
        break;
    }
}