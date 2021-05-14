#ifndef HEATING_H
#define HEATING_H

#include "util.h"

#define HEATING_ELEMENT_ALL (RELAY_1_PIN | RELAY_2_PIN | RELAY_3_PIN)
#define HEATING_ELEMENT_TOP RELAY_1_PIN
#define HEATING_ELEMENT_BOT RELAY_2_PIN

void heating_init(void);
void heating_on(uint32_t element, float power);
void heating_off(uint32_t element);

#endif // HEATING_H