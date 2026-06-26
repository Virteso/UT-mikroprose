#include <avr/io.h>
#include <avr/interrupt.h>
#include "7seg.h"

const uint8_t segment_map[] = {
    0b1111110,
    0b0110000,
    0b1101101,
    0b1111001,
    0b0110011,
    0b1011011,
    0b1011111,
    0b1110000,
    0b1111111,
    0b1111011,
    0b0000000
};
volatile uint8_t SEG_digits[2] = {10, 10};
volatile uint8_t current_digit = 0;

void shiftByteOut(uint8_t data) {
    PORTE &= ~(1 << PE4);
    // Pulse SHCP
    PORTE |= (1 << PE3);
    PORTE &= ~(1 << PE3);

    for (int i = 0; i < 7; i++, data >>= 1) {
        if (data & 1) {
            PORTE |= (1 << PE4);
        } else {
            PORTE &= ~(1 << PE4);
        }

        // Pulse SHCP
        PORTE |= (1 << PE3);
        PORTE &= ~(1 << PE3);
    }
    // Pulse STCP
    PORTB |= (1 << PB7);
    PORTB &= ~(1 << PB7);
}

ISR(TIMER1_COMPA_vect) {
    current_digit ^= 1;

    if (current_digit == 0) {
        shiftByteOut(segment_map[digits[0]]);
        PORTD |= (1 << PD4);
    } else {
        shiftByteOut(segment_map[digits[1]]);
        PORTD &= ~(1 << PD4);
    }
}

void SEGinitIO(){
    DDRE |= (1 << PE3) | (1 << PE4);
    DDRB |= (1 << PB7);
    DDRD |= (1 << PD4);
}

void SEGinitTimer(){
    TCNT1 = 0;
    OCR1A = 2000000 / (8 * 500);
    TIMSK1 = (1 << OCIE1A);
    TCCR1A = (0b11 << WGM10);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
}
