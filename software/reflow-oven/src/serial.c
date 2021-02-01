#include <stdint.h>
#include "cobs.h"
#include "ring_buffer.h"
#include "serial.h"
#include "usart2.h"

// These variables are declared volatile because they are modified by the USART interrupt.
static volatile uint32_t rx_buf_ovf = 0;
static volatile uint32_t delimiters = 0;
// Note that the ring buffer will only utilize CAPACITY - 1.
static volatile uint8_t tx_buf[SERIAL_BUFFER_LEN + 1];
static volatile uint8_t rx_buf[SERIAL_BUFFER_LEN + 1];
static ring_buffer_handle tx_buf_h = {sizeof(tx_buf), 0, 0, tx_buf};
static ring_buffer_handle rx_buf_h = {sizeof(rx_buf), 0, 0, rx_buf};

static uint32_t serial_tx_buf_pop(uint8_t *data)
{
    return ring_buffer_pop(&tx_buf_h, data);
}

static void serial_rx_buf_push(uint8_t data)
{
    if (ring_buffer_push(&rx_buf_h, data) == 0)
    {
        rx_buf_ovf++;
    }

    if (data == 0)
    {
        delimiters++;
    }
}

void serial_init(void)
{
    usart2_init(serial_tx_buf_pop, serial_rx_buf_push);
}

uint32_t serial_transmit(const uint8_t *packet, uint32_t len)
{
    uint32_t packet_enc_len = 0;
    uint32_t packet_enc_pos = 0;
    uint8_t packet_enc[SERIAL_BUFFER_LEN];

    packet_enc_len = cobs_encode(packet, len, packet_enc, SERIAL_BUFFER_LEN);

    while (packet_enc_pos < packet_enc_len)
    {
        packet_enc_pos += ring_buffer_push_n(&tx_buf_h, &packet_enc[packet_enc_pos], packet_enc_len - packet_enc_pos);
        usart2_tx_start();
    }

    ring_buffer_push(&tx_buf_h, 0);
    usart2_tx_start(); // Make sure the transmit interrupt is still enabled.

    return packet_enc_pos;
}

uint32_t serial_receive(uint8_t *packet, uint32_t max_len)
{
    uint8_t next_byte;
    uint32_t success = 0;
    uint32_t packet_enc_pos = 0;
    uint8_t packet_enc[SERIAL_BUFFER_LEN];
    struct cobs_result result;
    uint32_t len = 0;

    if (delimiters > 0)
    {
        success = ring_buffer_pop(&rx_buf_h, &next_byte);
        while (success && (next_byte != 0) && (packet_enc_pos < SERIAL_BUFFER_LEN))
        {
            packet_enc[packet_enc_pos++] = next_byte;
            success = ring_buffer_pop(&rx_buf_h, &next_byte);
        }

        if (!success)
        {
            // It appears that we have not found a delimiter (0x00) before
            // reaching the end of the receive buffer.
            while (1)
                ;
        }

        if ((packet_enc_pos >= SERIAL_BUFFER_LEN) && (next_byte != 0))
        {
            // It appears we have come across a packet that was too big and
            // now do not have enough space to decode the packet.
            while (1)
                ;
        }

        delimiters--;
        result = cobs_decode(packet_enc, packet_enc_pos, packet, max_len);

        if ((result.status == COBS_SRC_ZERO) || (result.status == COBS_SRC_UNDERFLOW))
        {
            // At this stage COBS_SRC_ZERO should not happen as we've only
            // extracted up to the first delimiter (0x00) however
            // COBS_SRC_UNDERFLOW could result from a malformed packet.
            while (1)
                ;
        }

        len = result.len;
    }

    return len;
}