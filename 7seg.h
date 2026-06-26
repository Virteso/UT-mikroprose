#ifndef SEVENSEG_H
#define SEVENSEG_H

#include <stdint.h>

extern const uint8_t segment_map[];
extern volatile uint8_t SEG_digits[2];
extern volatile uint8_t current_digit;

void shiftByteOut(uint8_t data);
void SEGinitTimer1();
void SEGinitIO();
void SEGinit();

#endif
