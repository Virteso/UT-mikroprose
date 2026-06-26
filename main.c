#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU 2000000
#endif
#include <util/delay.h>

#include "7seg.h"
#include "lcd.h"

int main(void)
{
	SEGinit();
	lcd_init();
	lcd_buffer[0][0] = 'H';
	lcd_buffer[0][1] = 'o';
	lcd_buffer[0][2] = 'l';
	lcd_buffer[0][3] = 'a';
	lcd_buffer[0][4] = '!';
	lcd_update();
	sei();
	while (1)
	{
		uint8_t val = 0;
		while (val < 100)
		{
			_delay_ms(500);
			val++;
			SEG_digits[0] = val % 10;
			SEG_digits[1] = val / 10;
		}
	}
}
