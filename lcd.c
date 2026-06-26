#include <avr/io.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 2000000
#endif
#include <util/delay.h>
#include "lcd.h"

#define LCD_RS PD7
#define LCD_RW PD6
#define LCD_E  PE0
#define BUSY_FLAG (1 << PC0)

#define RS_DDR  DDRD
#define RS_PORT PORTD
#define RW_DDR    DDRD
#define RW_PORT   PORTD
#define E_DDR     DDRE
#define E_PORT    PORTE

#define DATA_PORT PORTC
#define DATA_DDR  DDRC
#define DATA_PIN  PINC

volatile char lcd_buffer[LCD_ROWS][LCD_COLS];

static uint8_t lcd_reverse_bits(uint8_t b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

static void lcd_pulse(void) {
	E_PORT |= (1 << LCD_E);
	_delay_us(1); // ? vb pole vaja
	E_PORT &= ~(1 << LCD_E);
	_delay_us(40);
}

static void lcd_busy_wait(void) {
	RS_PORT &= ~(1 << LCD_RS);
	RW_PORT |= (1 << LCD_RW);
	DATA_PORT = 0;
	uint8_t busy = 1;
	while (busy) {
		E_PORT |= (1 << LCD_E);
		busy = DATA_PIN & BUSY_FLAG;
		E_PORT &= ~(1 << LCD_E);
		if (busy) _delay_us(80);
	}
}

void lcd_write_cmd(uint8_t cmd) {
	lcd_busy_wait();
	RS_PORT &= ~(1 << LCD_RS);
	RW_PORT &= ~(1 << LCD_RW);
	DATA_PORT = lcd_reverse_bits(cmd);
	lcd_pulse();
}

void lcd_write_data(uint8_t data) {
	lcd_busy_wait();
	RS_PORT |= (1 << LCD_RS);
	RW_PORT &= ~(1 << LCD_RW);
	DATA_PORT = lcd_reverse_bits(data);
	lcd_pulse();
}

void lcd_init(void) {
	_delay_ms(40);
	DATA_DDR = 0xFF;
	RS_DDR |= (1 << LCD_RS);
	RW_DDR  |= (1 << LCD_RW);
	E_DDR   |= (1 << LCD_E);

	lcd_write_cmd((1 << PC5) | (1 << PC4) | (1 << PC3)); // Function set: 8-bit, 2 lines, 5x8 font
	lcd_clear();
}

void lcd_clear(void) {
	for (uint8_t r = 0; r < LCD_ROWS; r++)
		for (uint8_t c = 0; c < LCD_COLS; c++)
			lcd_buffer[r][c] = ' ';
	lcd_write_cmd(0x01);
	_delay_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
	const uint8_t addr[] = {0x00, 0x40};
	lcd_write_cmd((1 << PC7) | (addr[row] + col));
}

void lcd_update(void) {
	for (uint8_t r = 0; r < LCD_ROWS; r++) {
		lcd_set_cursor(0, r);
		for (uint8_t c = 0; c < LCD_COLS; c++) {
			lcd_write_data(lcd_buffer[r][c]);
		}
	}
}
