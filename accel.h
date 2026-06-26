#ifndef ACCEL_H
#define ACCEL_H

#include <stdint.h>

void accel_init(void);
void accel_read(int16_t *x, int16_t *y, int16_t *z);
void accel_get_angles(int16_t x, int16_t y, int16_t z, float *pitch, float *roll);

#endif
