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
#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>
#include "pin.h"

void init_spi(void);

typedef struct {
    uint16_t *SPIxSTAT;
    uint16_t *SPIxCON1;
    uint16_t *SPIxCON2;
    uint16_t *SPIxBUF;
    uint16_t *MISOrpinr;
    uint8_t MISOrpshift;
    int16_t MOSIrpnum;
    int16_t SCKrpnum;
    _PIN *MISO;
    _PIN *MOSI;
    _PIN *SCK;
} _SPI;

extern _SPI spi1, spi2, spi3;

void spi_init(_SPI *self, uint16_t *SPIxSTAT, uint16_t *SPIxCON1, 
              uint16_t *SPIxCON2, uint16_t *SPIxBUF, 
              uint16_t *MISOrpinr, uint8_t MISOrpshift, 
              int16_t MOSIrpnum, int16_t SCKrpnum);
void spi_open(_SPI *self, _PIN *MISO, _PIN *MOSI, _PIN *SCK, float freq);
void spi_close(_SPI *self);
uint8_t spi_transfer(_SPI *self, uint8_t val);

#endif
