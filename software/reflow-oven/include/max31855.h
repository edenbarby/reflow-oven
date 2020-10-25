#ifndef MAX31855_H
#define MAX31855_H

enum MAX31855_STATUS
{
    MAX31855_OK = 0x00,
    MAX31855_ERROR_COMMS = 0x01,
    MAX31855_FAULT = 0x02,
    MAX31855_FAULT_OPEN = 0x03,
    MAX31855_FAULT_SHORT_GND = 0x04,
    MAX31855_FAULT_SHORT_VCC = 0x05
};

void max31855_init(void);
enum MAX31855_STATUS max31855_read(int16_t *temp_tc, int16_t *temp_ref);
float max31855_convert_temp_tc(int16_t temp_tc);
float max31855_convert_temp_ref(int16_t temp_ref);

#endif // MAX31855_H