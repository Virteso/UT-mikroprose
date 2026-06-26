#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

#include "ps2kbd.h"

#ifndef F_CPU
#define F_CPU 2000000UL
#endif

#define PS2_DATA_PIN  PIND
#define PS2_DATA_BIT  PD0
#define PS2_CLK_PIN   PIND
#define PS2_CLK_BIT   PD1

static volatile uint8_t rx_buffer[16];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static volatile uint8_t rx_overrun;
static volatile uint8_t clk_state;

static void ps2kbd_wait_for_low(void) {
    while ((PS2_CLK_PIN & (1 << PS2_CLK_BIT)) != 0) {
        ;
    }
}

static void ps2kbd_wait_for_high(void) {
    while ((PS2_CLK_PIN & (1 << PS2_CLK_BIT)) == 0) {
        ;
    }
}

static uint8_t ps2kbd_read_bit(void) {
    ps2kbd_wait_for_low();
    ps2kbd_wait_for_high();
    return (PS2_DATA_PIN >> PS2_DATA_BIT) & 0x01;
}

static void ps2kbd_enqueue(uint8_t value) {
    uint8_t next = (rx_head + 1) & 0x0F;
    if (next == rx_tail) {
        rx_overrun = 1;
        return;
    }
    rx_buffer[rx_head] = value;
    rx_head = next;
}

ISR(INT1_vect) {
    static uint8_t bit_count = 0;
    static uint8_t shift = 0;
    static uint8_t byte = 0;
    static uint8_t parity = 0;
    static uint8_t receiving = 0;

    if (receiving == 0) {
        if ((PS2_DATA_PIN & (1 << PS2_DATA_BIT)) == 0) {
            receiving = 1;
            bit_count = 0;
            shift = 0;
            byte = 0;
            parity = 0;
        }
        return;
    }

    if (bit_count < 8) {
        uint8_t bit = (PS2_DATA_PIN >> PS2_DATA_BIT) & 0x01;
        byte |= (bit << bit_count);
        parity ^= bit;
    } else if (bit_count == 8) {
        parity ^= 1;
        if ((PS2_DATA_PIN >> PS2_DATA_BIT) & 0x01) {
            /* parity ok */
        }
    } else if (bit_count == 9) {
        if (((PS2_DATA_PIN >> PS2_DATA_BIT) & 0x01) == 1) {
            ps2kbd_enqueue(byte);
        }
        receiving = 0;
        bit_count = 0;
        return;
    }

    bit_count++;
}

void ps2kbd_init(void) {
    DDRD &= ~(1 << PS2_DATA_BIT);
    DDRD &= ~(1 << PS2_CLK_BIT);
    PORTD |= (1 << PS2_DATA_BIT) | (1 << PS2_CLK_BIT);

    EICRA |= (1 << ISC11) | (1 << ISC10);
    EIMSK |= (1 << INT1);
    sei();
}

uint8_t ps2kbd_has_data(void) {
    return rx_head != rx_tail;
}

uint8_t ps2kbd_read_byte(uint8_t *data) {
    if (rx_head == rx_tail) {
        return 0;
    }

    *data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) & 0x0F;
    return 1;
}
