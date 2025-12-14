#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

extern uint8_t getbyte(void);
extern void sysvar_init(void);
extern void getblock(char *data, uint24_t length);
extern void putblock(char *data, uint24_t length);

#endif