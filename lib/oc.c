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
#include "oc.h"

_OC oc1, oc2, oc3, oc4, oc5, oc6, oc7, oc8, oc9;

uint16_t __pwmRead(_PIN *self) {
    WORD32 temp;
    _OC *oc;

    oc = (_OC *)(self->owner);
    temp.w[0] = 0;
    temp.w[1] = *(oc->OCxR);
    return (uint16_t)(temp.ul/(uint32_t)(*(oc->OCxRS)));
}

void __pwmWrite(_PIN *self, uint16_t val) {
    WORD32 temp;
    _OC *oc;

    oc = (_OC *)(self->owner);
    temp.ul = (uint32_t)val*(uint32_t)(*(oc->OCxRS));
    *(oc->OCxR) = temp.w[1];
}

uint16_t __servoRead(_PIN *self) {
    WORD32 temp;
    _OC *oc;

    oc = (_OC *)(self->owner);
    temp.w[0] = 0;
    temp.w[1] = *(oc->OCxRS)-oc->servooffset;
    return (uint16_t)(temp.ul/(uint32_t)(oc->servomultiplier));
}

void __servoWrite(_PIN *self, uint16_t val) {
    WORD32 temp;
    _OC *oc;

    oc = (_OC *)(self->owner);
    temp.ul = (uint32_t)val*(uint32_t)(oc->servomultiplier);
    *(oc->OCxRS) = oc->servooffset+temp.w[1];
}

void init_oc(void) {
    oc_init(&oc1, (uint16_t *)&OC1CON1, (uint16_t *)&OC1CON2, 
            (uint16_t *)&OC1RS, (uint16_t *)&OC1R, (uint16_t *)&OC1TMR, 18);
    oc_init(&oc2, (uint16_t *)&OC2CON1, (uint16_t *)&OC2CON2, 
            (uint16_t *)&OC2RS, (uint16_t *)&OC2R, (uint16_t *)&OC2TMR, 19);
    oc_init(&oc3, (uint16_t *)&OC3CON1, (uint16_t *)&OC3CON2, 
            (uint16_t *)&OC3RS, (uint16_t *)&OC3R, (uint16_t *)&OC3TMR, 20);
    oc_init(&oc4, (uint16_t *)&OC4CON1, (uint16_t *)&OC4CON2, 
            (uint16_t *)&OC4RS, (uint16_t *)&OC4R, (uint16_t *)&OC4TMR, 21);
    oc_init(&oc5, (uint16_t *)&OC5CON1, (uint16_t *)&OC5CON2, 
            (uint16_t *)&OC5RS, (uint16_t *)&OC5R, (uint16_t *)&OC5TMR, 22);
    oc_init(&oc6, (uint16_t *)&OC6CON1, (uint16_t *)&OC6CON2, 
            (uint16_t *)&OC6RS, (uint16_t *)&OC6R, (uint16_t *)&OC6TMR, 23);
    oc_init(&oc7, (uint16_t *)&OC7CON1, (uint16_t *)&OC7CON2, 
            (uint16_t *)&OC7RS, (uint16_t *)&OC7R, (uint16_t *)&OC7TMR, 24);
    oc_init(&oc8, (uint16_t *)&OC8CON1, (uint16_t *)&OC8CON2, 
            (uint16_t *)&OC8RS, (uint16_t *)&OC8R, (uint16_t *)&OC8TMR, 25);
    oc_init(&oc9, (uint16_t *)&OC9CON1, (uint16_t *)&OC9CON2, 
            (uint16_t *)&OC9RS, (uint16_t *)&OC9R, (uint16_t *)&OC9TMR, 35);
}

void oc_init(_OC *self, uint16_t *OCxCON1, uint16_t *OCxCON2, 
             uint16_t *OCxRS, uint16_t *OCxR, uint16_t *OCxTMR, 
             uint16_t rpnum) {
    self->OCxCON1 = OCxCON1;
    self->OCxCON2 = OCxCON2;
    self->OCxRS = OCxRS;
    self->OCxR = OCxR;
    self->OCxTMR = OCxTMR;
    self->servooffset = (uint16_t)(1e-3*FCY);
    self->servomultiplier = (uint16_t)(1e-3*FCY);
    self->rpnum = rpnum;
    self->pin = NULL;
}

void oc_free(_OC *self) {
    *(self->OCxCON1) = 0;
    *(self->OCxCON2) = 0;
    if (self->pin) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->pin->rpor) &= ~(0x3F<<(self->pin->rpshift));
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->pin->owner = NULL;
        pin_digitalOut(self->pin);
        pin_clear(self->pin);
        self->pin = NULL;
    }
}

void oc_pwm(_OC *self, _PIN *pin, _TIMER *timer, float freq, uint16_t duty) {
    WORD32 temp;

    if (pin->rpnum==-1)
        return;
    if (pin->owner==NULL) {
        self->pin = pin;
        pin->owner = (void *)self;
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(pin->rpor) &= ~(0x3F<<(pin->rpshift));
        *(pin->rpor) |= (self->rpnum)<<(pin->rpshift);
        __builtin_write_OSCCONL(OSCCON|0x40);
    } else if (pin->owner!=(void *)self) {
        return;
    }
    if (timer) {
        *(self->OCxCON1) = ((timer->octselnum)<<10)|0x0006;
        *(self->OCxCON2) = 0x001F;
        timer_setFreq(timer, freq);
        *(self->OCxRS) = *(timer->PRx);
        *(self->OCxTMR) = 0;
        timer_start(timer);
    } else {
        *(self->OCxCON1) = 0x1C06;
        *(self->OCxCON2) = 0x001F;
        if (freq<(FCY/65536.))
            *(self->OCxRS) = 0xFFFF;
        else
            *(self->OCxRS) = (uint16_t)(FCY/freq-1.);
    }
    temp.ul = (uint32_t)duty*(uint32_t)(*(self->OCxRS));
    *(self->OCxR) = temp.w[1];
    self->pin->write = __pwmWrite;
    self->pin->read = __pwmRead;
}

void oc_servo(_OC *self, _PIN *pin, _TIMER *timer, float interval, 
              float min_width, float max_width, uint16_t pos) {
    WORD32 temp;

    if (pin->rpnum==-1)
        return;
    if (pin->owner==NULL) {
        self->pin = pin;
        pin->owner = (void *)self;
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(pin->rpor) &= ~(0x3F<<(pin->rpshift));
        *(pin->rpor) |= (self->rpnum)<<(pin->rpshift);
        __builtin_write_OSCCONL(OSCCON|0x40);
    } else if (pin->owner!=(void *)self) {
        return;
    }
    if (timer) {
        self->servooffset = (uint16_t)(FCY*min_width);
        self->servomultiplier = (uint16_t)(FCY*(max_width-min_width));
        *(self->OCxCON1) = 0x1C0F;
        *(self->OCxCON2) = 0x0080|(timer->ocsyncselnum);
        *(self->OCxR) = 1;
        temp.ul = (uint32_t)pos*(uint32_t)(self->servomultiplier);
        *(self->OCxRS) = self->servooffset+temp.w[1];
        timer_setPeriod(timer, interval);
        timer_start(timer);
        self->pin->write = __servoWrite;
        self->pin->read = __servoRead;
    }
}

