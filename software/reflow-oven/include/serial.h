#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

// [type][data]
#define SERIAL_TYPE_ACK 0x00
#define SERIAL_TYPE_NACK 0x01
#define SERIAL_TYPE_QUERY 0x10
#define SERIAL_TYPE_RESP 0x11
#define SERIAL_TYPE_WAIT 0x20
#define SERIAL_TYPE_REFLOW 0x21

// [query][query sub type]
#define SERIAL_QUERY_STATE 0x00
#define SERIAL_QUERY_TPOVN 0x10
#define SERIAL_QUERY_TPCPU 0x11

void serial_init(void);
uint32_t serial_transmit(const uint8_t *buf, uint32_t len);
uint32_t serial_receive(uint8_t *buf, uint32_t len);
uint32_t serial_process(void);

#endif // SERIAL_H