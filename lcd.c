#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

#define LCD_RS PORTE1
#define LCD_RW PORTD6
#define LCD_E  PORTD7

#define CTRL_DDR  DDRE
#define CTRL_PORT PORTE
#define RW_DDR    DDRD
#define RW_PORT   PORTD
#define E_DDR     DDRD
#define E_PORT    PORTD

#define DATA_PORT PORTC
#define DATA_DDR  DDRC
#define DATA_PIN  PINC

volatile char lcd_buffer[LCD_ROWS][LCD_COLS];

static void lcd_pulse(void) {
    E_PORT |= (1 << LCD_E);
    _delay_us(1);
    E_PORT &= ~(1 << LCD_E);
    _delay_us(50);
}

static void lcd_busy_wait(void) {
    DATA_DDR &= 0x00;
    CTRL_PORT &= ~(1 << LCD_RS);
    RW_PORT |= (1 << LCD_RW);
    E_PORT |= (1 << LCD_E);
    _delay_us(1);
    while (DATA_PIN & 0x80) {
        E_PORT |= (1 << LCD_E);
        _delay_us(1);
        E_PORT &= ~(1 << LCD_E);
        _delay_us(1);
    }
    E_PORT &= ~(1 << LCD_E);
    RW_PORT &= ~(1 << LCD_RW);
    DATA_DDR = 0xFF;
}

void lcd_write_cmd(uint8_t cmd) {
    lcd_busy_wait();
    CTRL_PORT &= ~(1 << LCD_RS);
    RW_PORT &= ~(1 << LCD_RW);
    DATA_PORT = cmd;
    lcd_pulse();
}

void lcd_write_data(uint8_t data) {
    lcd_busy_wait();
    CTRL_PORT |= (1 << LCD_RS);
    RW_PORT &= ~(1 << LCD_RW);
    DATA_PORT = data;
    lcd_pulse();
}

void lcd_init(void) {
    _delay_ms(50);
    DATA_DDR = 0xFF;
    CTRL_DDR |= (1 << LCD_RS);
    E_DDR   |= (1 << LCD_E);
    RW_DDR  |= (1 << LCD_RW);

    CTRL_PORT &= ~(1 << LCD_RS);
    RW_PORT   &= ~(1 << LCD_RW);

    lcd_write_cmd(0x38);
    lcd_write_cmd(0x0E);
    lcd_write_cmd(0x06);
    lcd_write_cmd(0x01);
    _delay_ms(2);

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
    uint8_t addr[] = {0x00, 0x40};
    lcd_write_cmd(0x80 | (addr[row] + col));
}

void lcd_update(void) {
    for (uint8_t r = 0; r < LCD_ROWS; r++) {
        lcd_set_cursor(0, r);
        for (uint8_t c = 0; c < LCD_COLS; c++)
            lcd_write_data(lcd_buffer[r][c]);
    }
}
