#ifndef MAX31855_H
#define MAX31855_H


enum MAX31855_STATUS {
    MAX31855_OK,
    MAX31855_ERROR_COMMS,
    MAX31855_FAULT,
    MAX31855_FAULT_OPEN,
    MAX31855_FAULT_SHORT_GND,
    MAX31855_FAULT_SHORT_VCC
};


void max31855_init(void);
enum MAX31855_STATUS max31855_read(float * temp_tc, float * temp_ref);


#endif // MAX31855_H