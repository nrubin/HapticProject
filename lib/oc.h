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
#ifndef _OC_H_
#define _OC_H_

#include <stdint.h>
#include "pin.h"
#include "timer.h"

void init_oc(void);

typedef struct {
    uint16_t *OCxCON1;
    uint16_t *OCxCON2;
    uint16_t *OCxRS;
    uint16_t *OCxR;
    uint16_t *OCxTMR;
    uint16_t rpnum;
    uint16_t servooffset;
    uint16_t servomultiplier;
    _PIN *pin;
} _OC;

extern _OC oc1, oc2, oc3, oc4, oc5, oc6, oc7, oc8, oc9;

void oc_init(_OC *self, uint16_t *OCxCON1, uint16_t *OCxCON2, 
             uint16_t *OCxRS, uint16_t *OCxR, uint16_t *OCxTMR, 
             uint16_t rpnum);
void oc_free(_OC *self);
void oc_pwm(_OC *self, _PIN *out, _TIMER *timer, float freq, uint16_t duty);
void oc_servo(_OC *self, _PIN *out, _TIMER *timer, float interval, 
              float min_width, float max_width, uint16_t pos);

#endif
