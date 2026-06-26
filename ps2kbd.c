#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "ps2kbd.h"

#ifndef F_CPU
#define F_CPU 2000000UL
#endif

#define PS2_DATA_PIN  PIND
#define PS2_DATA_BIT  PD0
#define PS2_CLK_BIT   PD1

static volatile uint8_t rx_buffer[16];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static volatile uint8_t rx_overrun;

static volatile uint8_t ascii_buffer[16];
static volatile uint8_t ascii_head;
static volatile uint8_t ascii_tail;

static const uint8_t ps2_ascii_lut[256] = {
	['\x15'] = 'q',
	['\x1C'] = 'a',
	['\x1B'] = 's',
	['\x1D'] = 'w',
	['\x24'] = '7',
	['\x3B'] = 'j',
	['\x2D'] = 'o',
	['\x2E'] = 'p',
	['\x26'] = 'l',
	['\x1E'] = '1',
	['\x1F'] = '2',
	['\x20'] = '3',
	['\x21'] = '4',
	['\x22'] = '5',
	['\x23'] = '6',
	['\x25'] = '8',
	['\x46'] = '9',
	['\x27'] = '0',
	['\x29'] = ' ',
	['\x5A'] = '\r',
	['\x66'] = '\b',
};

static void ps2kbd_enqueue(uint8_t value) {
	uint8_t next = (rx_head + 1) & 0x0F;
	if (next == rx_tail) {
		rx_overrun = 1;
		return;
	}
	rx_buffer[rx_head] = value;
	rx_head = next;
}

static void ps2kbd_enqueue_ascii(uint8_t value) {
	uint8_t next = (ascii_head + 1) & 0x0F;
	if (next == ascii_tail)
		return;
	ascii_buffer[ascii_head] = value;
	ascii_head = next;
}

ISR(INT1_vect) {
	static uint8_t bit_count = 0;
	static uint8_t byte = 0;
	static uint8_t receiving = 0;

	if (receiving == 0) {
		if ((PS2_DATA_PIN & (1 << PS2_DATA_BIT)) == 0) {
			receiving = 1;
			bit_count = 0;
			byte = 0;
		}
		return;
	}

	if (bit_count < 8) {
		uint8_t bit = (PS2_DATA_PIN >> PS2_DATA_BIT) & 0x01;
		byte |= (bit << bit_count);
		bit_count++;
	} else if (bit_count == 8) {
		bit_count++;
	} else if (bit_count == 9) {
		if ((PS2_DATA_PIN & (1 << PS2_DATA_BIT)) != 0) {
			ps2kbd_enqueue(byte);
			if (byte < 0x80) {
				uint8_t ch = ps2_ascii_lut[byte];
				if (ch != 0)
					ps2kbd_enqueue_ascii(ch);
			}
		}
		receiving = 0;
	}
}

void ps2kbd_init(void) {
	DDRD &= ~((1 << PS2_DATA_BIT) | (1 << PS2_CLK_BIT));
	PORTD |= (1 << PS2_DATA_BIT) | (1 << PS2_CLK_BIT);

	EICRA |= (1 << ISC11);
	EICRA &= ~(1 << ISC10);
	EIMSK |= (1 << INT1);
	sei();
}

uint8_t ps2kbd_has_data(void) {
	return rx_head != rx_tail;
}

uint8_t ps2kbd_read_byte(uint8_t *data) {
	if (rx_head == rx_tail)
		return 0;

	*data = rx_buffer[rx_tail];
	rx_tail = (rx_tail + 1) & 0x0F;
	return 1;
}

uint8_t ps2kbd_has_ascii(void) {
	return ascii_head != ascii_tail;
}

uint8_t ps2kbd_read_ascii(uint8_t *data) {
	if (ascii_head == ascii_tail)
		return 0;

	*data = ascii_buffer[ascii_tail];
	ascii_tail = (ascii_tail + 1) & 0x0F;
	return 1;
}
