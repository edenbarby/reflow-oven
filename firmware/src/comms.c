#include <string.h>

#include "comms.h"
#include "serial.h"
#include "system.h"

#define COMMS_TYPE_NACK 0
#define COMMS_TYPE_CMD_IDLE 1
#define COMMS_TYPE_CMD_RUN 2
#define COMMS_TYPE_QUERY_ALL 3

static enum comms_state_e {
    COMMS_STATE_INIT,
    COMMS_STATE_RUN
} state = COMMS_STATE_INIT;

void comms_init(void)
{
    serial_init();
    state = COMMS_STATE_RUN;
}

void comms_run(oven_context_t *context)
{
    uint32_t rx_len = 0;
    uint32_t tx_len = 0;
    uint8_t packet_buf[SERIAL_PACKET_MAX_LEN] = {0};

    rx_len = serial_receive(packet_buf, SERIAL_PACKET_MAX_LEN);
    if (rx_len > 0)
    {
        switch (packet_buf[0])
        {
        case COMMS_TYPE_CMD_IDLE:
            if (rx_len == 1)
            {
                context->state = OVEN_STATE_IDLE;

                packet_buf[tx_len++] = COMMS_TYPE_CMD_IDLE;
                memcpy(&packet_buf[tx_len], &context->errors, sizeof(uint16_t));
                tx_len += sizeof(uint16_t);
            }
            break;
        case COMMS_TYPE_CMD_RUN:
            if (rx_len == (1 + sizeof(reflow_profile_t)))
            {
                context->state = OVEN_STATE_SOAK_RAMP;
                memcpy(&context->profile, &packet_buf[1], sizeof(reflow_profile_t));

                packet_buf[tx_len++] = COMMS_TYPE_CMD_RUN;
                memcpy(&packet_buf[tx_len], &context->errors, sizeof(uint16_t));
                tx_len += sizeof(uint16_t);
            }
            break;
        case COMMS_TYPE_QUERY_ALL:
            if (rx_len == 1)
            {
                packet_buf[tx_len++] = COMMS_TYPE_QUERY_ALL;
                memcpy(&packet_buf[tx_len], context, sizeof(oven_context_t));
                tx_len += sizeof(oven_context_t);
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
            {
            }
        }
    }
}

void comms_task(oven_context_t *context)
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