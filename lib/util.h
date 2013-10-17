#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>

#define FCY     16e6
#define TCY     62.5e-9

#ifndef NULL
#define NULL 0
#endif

#define peek(addr)              *(addr)
#define poke(addr, val)         *(addr) = val
#define bitread(addr, bit)      (((*(addr))&(1<<bit)) ? 1:0)
#define bitset(addr, bit)       *(addr) |= 1<<bit
#define bitclear(addr, bit)     *(addr) &= ~(1<<bit)
#define bitflip(addr, bit)      *(addr) ^= 1<<bit

typedef unsigned char BYTE;

typedef union {
    int16_t i;
    uint16_t w;
    uint8_t b[2];
} WORD;

typedef union {
    int32_t l;
    uint32_t ul;
    uint16_t w[2];
    uint8_t b[4];
} WORD32;

#endif
