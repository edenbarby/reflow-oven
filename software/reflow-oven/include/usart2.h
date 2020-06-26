#ifndef USART2_H
#define USART2_H


#include <stdint.h>


void usart2_init(void);
void usart2_putc(uint8_t data);
uint8_t usart2_getc(uint8_t * data);


#endif // USART2_H