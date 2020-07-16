#ifndef USART2_H
#define USART2_H

#include <stdint.h>

void usart2_init(void);
uint32_t usart2_tx_available(void);
uint32_t usart2_rx_available(void);
void usart2_tx_n(const uint8_t *data, uint32_t n);
void usart2_rx_n(uint8_t *data, uint32_t n);

#endif // USART2_H