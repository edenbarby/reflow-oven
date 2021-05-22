#ifndef USART2_H
#define USART2_H

#include <stdint.h>

void usart2_init(uint32_t (*tx_pop)(uint8_t *), void (*rx_push)(uint8_t));
void usart2_tx_start(void);
void usart2_clear_overrun(void);
void usart2_clear_noise(void);
void usart2_clear_frame(void);
uint32_t usart2_get_overrun(void);
uint32_t usart2_get_noise(void);
uint32_t usart2_get_frame(void);

#endif // USART2_H