#ifndef SEVENSEG_H
#define SEVENSEG_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

const uint8_t segment_map[11];
volatile uint8_t SEG_digits[2];
uint8_t current_digit;

void shiftByteOut(uint8_t data);
void SEGinitTimer1();
void SEGinitIO();
void SEGinit();

#endif
