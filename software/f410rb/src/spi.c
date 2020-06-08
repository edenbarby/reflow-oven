// #include <spi.h>
//
//
// // SPI2
// #define SPI_PORT     GPIOB
// #define SPI_PIN_SCK  LL_GPIO_PIN_13
// #define SPI_PIN_MISO LL_GPIO_PIN_14
// #define SPI_PIN_MOSI LL_GPIO_PIN_15
//
//
// void spi_init(void) {
//     LL_GPIO_InitTypeDef init_struct_gpio;
//     LL_SPI_InitTypeDef init_struct_spi;
//
//     LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
//     LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
//
//     init_struct_gpio.Alternate  = LL_GPIO_AF_5;
//     init_struct_gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
//     init_struct_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//     init_struct_gpio.Pin        = LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
//     init_struct_gpio.Pull       = LL_GPIO_PULL_NO;
//     init_struct_gpio.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
//     LL_GPIO_Init(GPIOB, &init_struct_gpio);
//
//     init_struct_spi.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV256;
//     init_struct_spi.BitOrder          = LL_SPI_MSB_FIRST;
//     init_struct_spi.ClockPhase        = LL_SPI_PHASE_1EDGE;
//     init_struct_spi.ClockPolarity     = LL_SPI_POLARITY_LOW;
//     init_struct_spi.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
//     init_struct_spi.CRCPoly           = 15;
//     init_struct_spi.DataWidth         = LL_SPI_DATAWIDTH_16BIT;
//     init_struct_spi.Mode              = LL_SPI_MODE_MASTER;
//     init_struct_spi.NSS               = LL_SPI_NSS_SOFT;
//     init_struct_spi.TransferDirection = LL_SPI_FULL_DUPLEX;
//     LL_SPI_Init(SPI2, &init_struct_spi);
//     LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
// }
//
// uint8_t  spi_transceive_8(uint8_t data);
// uint16_t spi_transceive_16(uint16 data);
// void     spi_transceive_8n(uint8_t *data_tx, uint8_t *data_rx uint32_t len);
// void     spi_transceive_16n(uint16_t *data_tx, uint16_t *data_rx, uint32_t len);
