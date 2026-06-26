#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "accel.h"

#define LIS3DH_WHO_AM_I     0x0F
#define LIS3DH_CTRL_REG1    0x20
#define LIS3DH_CTRL_REG4    0x23
#define LIS3DH_OUT_X_L      0x28

#define ACC_CS_PORT PORTB
#define ACC_CS_DDR  DDRB
#define ACC_CS_PIN  PB0

#define SPI_READ(addr)   (0x80 | (addr))
#define SPI_WRITE(addr)  ((addr) & 0x3F)
#define SPI_READ_MULTI(addr) (0xC0 | (addr))

static void cs_low(void) {
    ACC_CS_PORT &= ~(1 << ACC_CS_PIN);
}

static void cs_high(void) {
    ACC_CS_PORT |= (1 << ACC_CS_PIN);
}

static uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}

static uint8_t accel_read_reg(uint8_t reg) {
    cs_low();
    spi_transfer(SPI_READ(reg));
    uint8_t val = spi_transfer(0);
    cs_high();
    return val;
}

static void accel_write_reg(uint8_t reg, uint8_t val) {
    cs_low();
    spi_transfer(SPI_WRITE(reg));
    spi_transfer(val);
    cs_high();
}

static void accel_read_multi(uint8_t reg, uint8_t *buf, uint8_t len) {
    cs_low();
    spi_transfer(SPI_READ_MULTI(reg));
    for (uint8_t i = 0; i < len; i++)
        buf[i] = spi_transfer(0);
    cs_high();
}

void accel_init(void) {
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
    DDRB &= ~(1 << PB3);

    SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA);
    SPSR = 0;

    cs_high();
    _delay_ms(5);

    if (accel_read_reg(LIS3DH_WHO_AM_I) != 0x33)
        return;

    accel_write_reg(LIS3DH_CTRL_REG1, 0x57);
    accel_write_reg(LIS3DH_CTRL_REG4, 0x80);

    _delay_ms(10);
}

void accel_read(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buf[6];
    accel_read_multi(LIS3DH_OUT_X_L, buf, 6);

    *x = (int16_t)(buf[1] << 8 | buf[0]);
    *y = (int16_t)(buf[3] << 8 | buf[2]);
    *z = (int16_t)(buf[5] << 8 | buf[4]);
}

void accel_get_angles(int16_t x, int16_t y, int16_t z, float *pitch, float *roll) {
    float fx = (float)x / 16384.0f;
    float fy = (float)y / 16384.0f;
    float fz = (float)z / 16384.0f;

    *pitch = atan2f(-fx, sqrtf(fy * fy + fz * fz)) * 180.0f / M_PI;
    *roll  = atan2f(fy, fz) * 180.0f / M_PI;
}
