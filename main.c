#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 2000000
#endif
#include <util/delay.h>

#include "7seg.h"
#include "lcd.h"
#include "ps2kbd.h"

int main(void)
{
    SEGinit();
    lcd_init();
    ps2kbd_init();

    lcd_clear();
    lcd_buffer[0][0] = 'P';
    lcd_buffer[0][1] = 'S';
    lcd_buffer[0][2] = '2';
    lcd_buffer[0][3] = ':';
    lcd_update();

    sei();

    uint8_t col = 5;
    while (1)
    {
        uint8_t ch = 0;
        if (ps2kbd_read_ascii(&ch)) {
            if (ch == '\b') {
                if (col > 5) {
                    col--;
                    lcd_buffer[0][col] = ' ';
                }
            } else if (ch == '\r') {
                col = 5;
            } else {
                if (col < LCD_COLS) {
                    lcd_buffer[0][col] = (char)ch;
                    col++;
                }
            }
            lcd_update();
        }

        SEG_digits[0] = 0;
        SEG_digits[1] = 0;
        _delay_ms(5);
    }
}
