/*
** Copyright (c) 2013, Bradley A. Minch
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met: 
** 
**     1. Redistributions of source code must retain the above copyright 
**        notice, this list of conditions and the following disclaimer. 
**     2. Redistributions in binary form must reproduce the above copyright 
**        notice, this list of conditions and the following disclaimer in the 
**        documentation and/or other materials provided with the distribution. 
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
** POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _PIN_H_
#define _PIN_H_

#include <stdint.h>

void init_pin(void);

typedef struct _PIN {
    uint16_t *port;
    uint16_t *tris;
    uint16_t *ans;
    uint8_t bit;
    int16_t annum;
    uint8_t rpshift;
    int16_t rpnum;
    uint16_t *rpor;
    void *owner;
    void (*write)(struct _PIN *self, uint16_t val);
    uint16_t (*read)(struct _PIN *self);
} _PIN;

extern _PIN D[14], A[6];

void pin_init(_PIN *self, uint16_t *port, uint16_t *tris, 
              uint16_t *ans, uint8_t bit, int16_t annum, 
              uint8_t rpshift, uint16_t rpnum, uint16_t *rpor);
void pin_digitalIn(_PIN *self);
void pin_digitalOut(_PIN *self);
void pin_analogIn(_PIN *self);
void pin_set(_PIN *self);
void pin_clear(_PIN *self);
void pin_toggle(_PIN *self);
void pin_write(_PIN *self, uint16_t val);
uint16_t pin_read(_PIN *self);

#endif
