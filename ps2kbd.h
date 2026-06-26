#ifndef PS2KBD_H
#define PS2KBD_H

#include <stdint.h>

void ps2kbd_init(void);
uint8_t ps2kbd_read_byte(uint8_t *data);
uint8_t ps2kbd_has_data(void);
uint8_t ps2kbd_read_ascii(uint8_t *data);
uint8_t ps2kbd_has_ascii(void);

#endif
