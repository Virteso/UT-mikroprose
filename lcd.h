#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_COLS 16
#define LCD_ROWS 2

extern volatile char lcd_buffer[LCD_ROWS][LCD_COLS];

void lcd_init(void);
void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);
void lcd_update(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);

#endif
