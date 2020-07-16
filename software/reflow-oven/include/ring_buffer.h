#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

typedef struct
{
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
    uint8_t *buf;
} ring_buffer_handle;

uint32_t ring_buffer_empty(ring_buffer_handle *h);
uint32_t ring_buffer_full(ring_buffer_handle *h);
uint32_t ring_buffer_used(ring_buffer_handle *h);
uint32_t ring_buffer_remaining(ring_buffer_handle *h);
uint32_t ring_buffer_push(ring_buffer_handle *h, uint8_t data);
uint32_t ring_buffer_push_n(ring_buffer_handle *h, const uint8_t *data, uint32_t n);
uint32_t ring_buffer_pop(ring_buffer_handle *h, uint8_t *data);
uint32_t ring_buffer_pop_n(ring_buffer_handle *h, uint8_t *data, uint32_t n);

#endif // RING_BUFFER_H