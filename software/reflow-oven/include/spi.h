#ifndef SPI2_H
#define SPI2_H


#include <stdint.h>
#include "stm32f3xx_ll_spi.h"


struct spi_config {
    uint32_t bit_order;
    uint32_t prescaler;
    uint32_t clock_polarity;
    uint32_t clock_phase;
};


uint32_t spi_init(struct spi_config * h, uint32_t baud_rate_max);
uint32_t spi_transceive(struct spi_config * h, const uint8_t * data_tx, uint8_t * data_rx, uint32_t n);
uint32_t spi_transmit(struct spi_config * h, const uint8_t * data_tx, uint32_t n);
uint32_t spi_receive(struct spi_config * h, uint8_t * data_rx, uint32_t n);


#endif // SPI2_H