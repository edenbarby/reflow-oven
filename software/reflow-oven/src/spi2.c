#include "stm32f3xx_ll_bus.h"
#include "stm32f3xx_ll_gpio.h"
#include "stm32f3xx_ll_spi.h"
#include "spi2.h"
#include "util.h"


uint8_t spi2_transaction(uint8_t data);


void spi2_init(void) {
    LL_GPIO_InitTypeDef init_struct_gpio;
    LL_SPI_InitTypeDef  init_struct_spi;

    // Init GPIO.
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    init_struct_gpio.Alternate  = LL_GPIO_AF_5;
    init_struct_gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    init_struct_gpio.Pin        = SPI2_SCLK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN;
    init_struct_gpio.Pull       = LL_GPIO_PULL_UP;
    init_struct_gpio.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(SPI2_PORT, &init_struct_gpio);

    // Init SPI.
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
    init_struct_spi.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV32;
    init_struct_spi.BitOrder          = LL_SPI_MSB_FIRST;
    init_struct_spi.ClockPhase        = LL_SPI_PHASE_2EDGE;
    init_struct_spi.ClockPolarity     = LL_SPI_POLARITY_HIGH;
    init_struct_spi.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
    init_struct_spi.CRCPoly           = 0;
    init_struct_spi.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
    init_struct_spi.Mode              = LL_SPI_MODE_MASTER;
    init_struct_spi.NSS               = LL_SPI_NSS_SOFT;
    init_struct_spi.TransferDirection = LL_SPI_FULL_DUPLEX;
    LL_SPI_Init(SPI2, &init_struct_spi);
    SPI2->CR2 |= SPI_CR2_FRXTH;
}

uint8_t spi2_transaction(uint8_t data) {
    LL_SPI_TransmitData8(SPI2, data);
    while(LL_SPI_GetTxFIFOLevel(SPI2) != LL_SPI_TX_FIFO_EMPTY);
    while(LL_SPI_IsActiveFlag_BSY(SPI2));
    while(LL_SPI_GetRxFIFOLevel(SPI2) != LL_SPI_RX_FIFO_EMPTY) {
        data = LL_SPI_ReceiveData8(SPI2);
    }
    return data;
}

void spi2_receive(uint8_t * data_rx, uint32_t len) {
    uint32_t i;

    LL_SPI_Enable(SPI2);
    for(i = 0; i < len; i++) {
        data_rx[i] = spi2_transaction(0xFF);
    }
    LL_SPI_Disable(SPI2);
}

void spi2_transmit(const uint8_t * data_tx, uint32_t len) {
    uint32_t i;

    LL_SPI_Enable(SPI2);
    for(i = 0; i < len; i++) {
        spi2_transaction(data_tx[i]);
    }
    LL_SPI_Disable(SPI2);
}

void spi2_transceive(uint8_t * data_rx, const uint8_t * data_tx, uint32_t len) {
    uint32_t i;

    LL_SPI_Enable(SPI2);
    for(i = 0; i < len; i++) {
        data_rx[i] = spi2_transaction(data_tx[i]);
    }
    LL_SPI_Disable(SPI2);
}