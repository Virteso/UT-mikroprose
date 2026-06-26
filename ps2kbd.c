#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

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

static volatile uint8_t ascii_buffer[16];
static volatile uint8_t ascii_head;
static volatile uint8_t ascii_tail;

static volatile uint8_t shift_state;
static volatile uint8_t caps_state;
static volatile uint8_t release_state;

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
    if (next == ascii_tail) {
        return;
    }
    ascii_buffer[ascii_head] = value;
    ascii_head = next;
}

static void ps2kbd_process_scan_code(uint8_t scan) {
    if (scan == 0xF0) {
        release_state = 1;
        return;
    }

    if (scan == 0xE0) {
        return;
    }

    if (release_state) {
        if (scan == 0x12 || scan == 0x59) {
            shift_state = 0;
        }
        release_state = 0;
        return;
    }

    if (scan == 0x12 || scan == 0x59) {
        shift_state = 1;
        return;
    }

    if (scan == 0x58) {
        caps_state ^= 1;
        return;
    }

    switch (scan) {
        case 0x29:
            ps2kbd_enqueue_ascii(' ');
            break;
        case 0x5A:
            ps2kbd_enqueue_ascii('\r');
            break;
        case 0x66:
            ps2kbd_enqueue_ascii('\b');
            break;
        case 0x15:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'Q' : 'q');
            break;
        case 0x1C:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'A' : 'a');
            break;
        case 0x1B:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'S' : 's');
            break;
        case 0x1D:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'W' : 'w');
            break;
        case 0x24:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'J' : 'j');
            break;
        case 0x2D:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'O' : 'o');
            break;
        case 0x2E:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'P' : 'p');
            break;
        case 0x26:
            ps2kbd_enqueue_ascii(shift_state ^ caps_state ? 'L' : 'l');
            break;
        case 0x1E:
            ps2kbd_enqueue_ascii(shift_state ? '!' : '1');
            break;
        case 0x1F:
            ps2kbd_enqueue_ascii(shift_state ? '@' : '2');
            break;
        case 0x20:
            ps2kbd_enqueue_ascii(shift_state ? '#' : '3');
            break;
        case 0x21:
            ps2kbd_enqueue_ascii(shift_state ? '$' : '4');
            break;
        case 0x22:
            ps2kbd_enqueue_ascii(shift_state ? '%' : '5');
            break;
        case 0x23:
            ps2kbd_enqueue_ascii(shift_state ? '^' : '6');
            break;
        case 0x24:
            ps2kbd_enqueue_ascii(shift_state ? '&' : '7');
            break;
        case 0x25:
            ps2kbd_enqueue_ascii(shift_state ? '*' : '8');
            break;
        case 0x26:
            ps2kbd_enqueue_ascii(shift_state ? '(' : '9');
            break;
        case 0x27:
            ps2kbd_enqueue_ascii(shift_state ? ')' : '0');
            break;
        default:
            break;
    }
}

ISR(PCINT2_vect) {
    static uint8_t bit_count = 0;
    static uint8_t byte = 0;
    static uint8_t parity = 0;
    static uint8_t receiving = 0;

    if ((PS2_CLK_PIN & (1 << PS2_CLK_BIT)) == 0) {
        return;
    }

    if (receiving == 0) {
        if ((PS2_DATA_PIN & (1 << PS2_DATA_BIT)) == 0) {
            receiving = 1;
            bit_count = 0;
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
    } else if (bit_count == 9) {
        if (((PS2_DATA_PIN >> PS2_DATA_BIT) & 0x01) == 1) {
            ps2kbd_enqueue(byte);
            ps2kbd_process_scan_code(byte);
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

    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PS2_CLK_BIT);
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

uint8_t ps2kbd_has_ascii(void) {
    return ascii_head != ascii_tail;
}

uint8_t ps2kbd_read_ascii(uint8_t *data) {
    if (ascii_head == ascii_tail) {
        return 0;
    }

    *data = ascii_buffer[ascii_tail];
    ascii_tail = (ascii_tail + 1) & 0x0F;
    return 1;
}
