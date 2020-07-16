#include <stdint.h>
#include "ring_buffer.h"

uint32_t ring_buffer_empty(ring_buffer_handle *h)
{
    return h->head == h->tail;
}

uint32_t ring_buffer_full(ring_buffer_handle *h)
{
    return (h->head + 1) % h->capacity == h->tail;
}

uint32_t ring_buffer_used(ring_buffer_handle *h)
{
    return ((h->head + h->capacity) - h->tail) % h->capacity;
}

uint32_t ring_buffer_remaining(ring_buffer_handle *h)
{
    return h->capacity - ring_buffer_used(h) - 1;
}

uint32_t ring_buffer_push(ring_buffer_handle *h, uint8_t data)
{
    uint32_t head_next = (h->head + 1) % h->capacity;

    if (head_next != h->tail)
    {
        h->buf[h->head] = data;
        h->head = head_next;
    }
    else
    {
        return 0;
    }

    return 1;
}

uint32_t ring_buffer_push_n(ring_buffer_handle *h, const uint8_t *data, uint32_t n)
{
    uint32_t i = 0;
    uint32_t head_next = (h->head + 1) % h->capacity;

    while ((head_next != h->tail) && (i < n))
    {
        h->buf[h->head] = data[i++];
        h->head = head_next;
        head_next = (h->head + 1) % h->capacity;
    }

    return i;
}

uint32_t ring_buffer_pop(ring_buffer_handle *h, uint8_t *data)
{
    uint32_t i = 0;

    if (h->tail != h->head)
    {
        data[i++] = h->buf[h->tail];
        h->tail = (h->tail + 1) % h->capacity;
    }

    return i;
}

uint32_t ring_buffer_pop_n(ring_buffer_handle *h, uint8_t *data, uint32_t n)
{
    uint32_t i = 0;

    while ((h->tail != h->head) && (i < n))
    {
        data[i++] = h->buf[h->tail];
        h->tail = (h->tail + 1) % h->capacity;
    }

    return i;
}