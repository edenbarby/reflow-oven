#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void lcd_init(void);
void lcd_update(void);
void lcd_clear(void);
void lcd_set(void);
void lcd_set_cursor(uint8_t x, uint8_t y);
void lcd_print(const uint8_t *string, uint32_t len);
void lcd_print_c(uint8_t c);

#endif // LCD_H