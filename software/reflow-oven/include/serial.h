#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include "cobs.h"

#define SERIAL_BUFFER_LEN 300
#define SERIAL_PACKET_MAX_LEN CODS_DECODE_BUF_MAX(SERIAL_BUFFER_LEN)

void serial_init(void);
uint32_t serial_transmit(const uint8_t *packet, uint32_t len);
uint32_t serial_receive(uint8_t *packet, uint32_t max_len);

#endif // SERIAL_H