#ifndef SPI2_H
#define SPI2_H


#include <stdint.h>


void spi2_init(void);
void spi2_receive(uint8_t * data_rx, uint32_t len);
void spi2_transmit(const uint8_t * data_tx, uint32_t len);
void spi2_transceive(uint8_t * data_rx, const uint8_t * data_tx, uint32_t len);


#endif // SPI2_H