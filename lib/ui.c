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
#include <p24FJ128GB206.h>
#include "common.h"
#include "ui.h"

_LED led1, led2, led3;
_SW sw1, sw2, sw3;

void init_ui(void) {
    led_init(&led1, (uint16_t *)&PORTD, (uint16_t *)&TRISD, 
             (uint16_t *)&ANSD, 7);
    led_init(&led2, (uint16_t *)&PORTF, (uint16_t *)&TRISF, 
             (uint16_t *)&ANSF, 0);
    led_init(&led3, (uint16_t *)&PORTF, (uint16_t *)&TRISF, 
             (uint16_t *)&ANSF, 1);
    sw_init(&sw1, (uint16_t *)&PORTC, (uint16_t *)&TRISC, 
            (uint16_t *)&ANSC, 15);
    sw_init(&sw2, (uint16_t *)&PORTC, (uint16_t *)&TRISC, 
            (uint16_t *)&ANSC, 12);
    sw_init(&sw3, (uint16_t *)&PORTB, (uint16_t *)&TRISB, 
            (uint16_t *)&ANSB, 12);
}

void led_init(_LED *self, uint16_t *port, uint16_t *tris, 
              uint16_t *ans, uint8_t bit) {
    self->port = port;
    self->tris = tris;
    self->ans = ans;
    self->bit = bit;

    bitclear(self->ans, self->bit);
    bitclear(self->port, self->bit);
    bitclear(self->tris, self->bit);
}

void led_on(_LED *self) {
    bitset(self->port, self->bit);
}

void led_off(_LED *self) {
    bitclear(self->port, self->bit);
}

void led_toggle(_LED *self) {
    bitflip(self->port, self->bit);
}

void led_write(_LED *self, uint16_t val) {
    if (val)
        bitset(self->port, self->bit);
    else
        bitclear(self->port, self->bit);
}

uint16_t led_read(_LED *self) {
    return bitread(self->port, self->bit);
}

void sw_init(_SW *self, uint16_t *port, uint16_t *tris, 
             uint16_t *ans, uint8_t bit) {
    self->port = port;
    self->tris = tris;
    self->ans = ans;
    self->bit = bit;

    bitclear(self->ans, self->bit);
    bitset(self->tris, self->bit);
}

uint16_t sw_read(_SW *self) {
    return bitread(self->port, self->bit);
}
