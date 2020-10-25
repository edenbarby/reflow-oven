#include <stdint.h>
#include "cobs.h"
#include "ring_buffer.h"
#include "serial.h"
#include "usart2.h"

// Note that the ring buffer will only utilize CAPACITY - 1.
#define TX_BUF_LEN 300
#define RX_BUF_LEN 300

// These variables are declared volatile because they are modified by the USART interrupt in some way.
static volatile uint32_t rx_buf_ovf = 0;
static volatile uint32_t delimiters = 0;
static volatile uint8_t tx_buf[TX_BUF_LEN];
static volatile uint8_t rx_buf[RX_BUF_LEN];
static ring_buffer_handle tx_buf_h = {sizeof(tx_buf), 0, 0, tx_buf};
static ring_buffer_handle rx_buf_h = {sizeof(rx_buf), 0, 0, rx_buf};

static uint32_t serial_tx_buf_pop(uint8_t *data)
{
    return ring_buffer_pop(&tx_buf_h, data);
}

static void serial_rx_buf_push(uint8_t data)
{
    if(ring_buffer_push(&rx_buf_h, data) == 0)
    {
        rx_buf_ovf++;
    }

    if(data == 0)
    {
        delimiters++;
    }
}

void serial_init(void)
{
    usart2_init(serial_tx_buf_pop, serial_rx_buf_push);
}

uint32_t serial_transmit(const uint8_t *data, uint32_t length)
{
    uint32_t encoded_length, encoded_i;
    uint8_t encoded[300];

    encoded_length = cobs_encode(data, length, encoded, 300);
    encoded_i = 0;
    while(encoded_i < encoded_length)
    {
        encoded_i += ring_buffer_push_n(&tx_buf_h, &encoded[encoded_i], encoded_length - encoded_i);
        usart2_tx_start();
    }
    ring_buffer_push(&tx_buf_h, 0);
    return encoded_i;
}

uint32_t serial_receive(uint8_t *data, uint32_t length)
{
    uint8_t b;
    uint32_t n, encoded_i;
    uint8_t encoded[300];
    struct cobs_result result;

    if (delimiters > 0)
    {
        n = ring_buffer_pop(&rx_buf_h, &b);
        encoded_i = 0;
        while((n != 0) && (b != 0) && (encoded_i < 300))
        {
            encoded[encoded_i++] = b;
            n = ring_buffer_pop(&rx_buf_h, &b);
        }

        if((n == 0) || (encoded_i >= 300))
        {
            while(1); // TODO: Neither of these should happen, sort it out haha
        }

        delimiters--;
        result = cobs_decode(encoded, encoded_i, data, length);
        if((result.status == COBS_SRC_ZERO) || (result.status == COBS_SRC_UNDERFLOW))
        {
            while(1); // TODO: this shouldn't happen either lol
        }
        return result.len;
    }
    else
    {
        return 0;
    }
    
}

uint32_t serial_process(void)
{
    return 0;
}