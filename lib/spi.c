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
#include "spi.h"

_SPI spi1, spi2, spi3;

void init_spi(void) {
    spi_init(&spi1, (uint16_t *)&SPI1STAT, (uint16_t *)&SPI1CON1, 
             (uint16_t *)&SPI1CON2, (uint16_t *)&SPI1BUF, 
             (uint16_t *)&RPINR20, 0, 7, 8);
    spi_init(&spi2, (uint16_t *)&SPI2STAT, (uint16_t *)&SPI2CON1, 
             (uint16_t *)&SPI2CON2, (uint16_t *)&SPI2BUF, 
             (uint16_t *)&RPINR22, 0, 10, 11);
    spi_init(&spi3, (uint16_t *)&SPI3STAT, (uint16_t *)&SPI3CON1, 
             (uint16_t *)&SPI3CON2, (uint16_t *)&SPI3BUF, 
             (uint16_t *)&RPINR28, 0, 32, 33);
}

void spi_init(_SPI *self, uint16_t *SPIxSTAT, uint16_t *SPIxCON1, 
              uint16_t *SPIxCON2, uint16_t *SPIxBUF, 
              uint16_t *MISOrpinr, uint8_t MISOrpshift, 
              int16_t MOSIrpnum, int16_t SCKrpnum) {
    self->SPIxSTAT = SPIxSTAT;
    self->SPIxCON1 = SPIxCON1;
    self->SPIxCON2 = SPIxCON2;
    self->SPIxBUF = SPIxBUF;
    self->MISOrpinr = MISOrpinr;
    self->MISOrpshift = MISOrpshift;
    self->MOSIrpnum = MOSIrpnum;
    self->SCKrpnum = SCKrpnum;
    self->MISO = NULL;
    self->MOSI = NULL;
    self->SCK = NULL;
}

void spi_open(_SPI *self, _PIN *MISO, _PIN *MOSI, _PIN *SCK, float freq) {
    uint16_t primary, secondary;

    if ((MISO->rpnum==-1) || (MOSI->rpnum==-1) || (SCK->rpnum==-1))
        return; // At least one of the specified pins is not an RP pin
    if ((MISO->owner==NULL) && (MOSI->owner==NULL) && (SCK->owner==NULL)) {
        // All of the specified pins are available and RP pins, so configure 
        // as specified
        pin_digitalIn(MISO);
        pin_digitalOut(MOSI);
        pin_set(MOSI);
        pin_digitalOut(SCK);
        pin_clear(SCK);
        self->MISO = MISO;
        MISO->owner = (void *)self;
        MISO->write = NULL;
        MISO->read = NULL;
        self->MOSI = MOSI;
        MOSI->owner = (void *)self;
        MOSI->write = NULL;
        MOSI->read = NULL;
        self->SCK = SCK;
        SCK->owner = (void *)self;
        SCK->write = NULL;
        SCK->read = NULL;
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->MISOrpinr) &= ~(0x3F<<(self->MISOrpshift));
        *(self->MISOrpinr) |= (MISO->rpnum)<<(self->MISOrpshift);
        *(MOSI->rpor) &= ~(0x3F<<(MOSI->rpshift));
        *(MOSI->rpor) |= (self->MOSIrpnum)<<(MOSI->rpshift);
        *(SCK->rpor) &= ~(0x3F<<(SCK->rpshift));
        *(SCK->rpor) |= (self->SCKrpnum)<<(SCK->rpshift);
        __builtin_write_OSCCONL(OSCCON|0x40);
    } else if ((self->MISO!=MISO) || (self->MOSI!=MOSI) || (self->SCK!=SCK)) {
        return; // At least one of the specified pins does not match the 
                // previous assignment
    }
    // Clip freq to be in allowable range of values
    if (freq>(FCY/2.))
        freq = FCY/2.;
    if (freq<(FCY/(64.*8.)))
        freq = FCY/(64.*8.);
    // Select primary prescale bits
    if (freq<=(FCY/(2.*64.))) {
        freq *= 64.;
        primary = 0;    // Set primary prescale bits for 64:1
    } else if (freq<=(FCY/(2.*16.))) {
        freq *= 16.;
        primary = 1;    // Set primary prescale bits for 16:1
    } else if (freq<=(FCY/(2.*4.))) {
        freq *= 4.;
        primary = 2;    // Set primary prescale bits for 4:1
    } else {
        primary = 3;    // Set primary prescale bits for 1:1
    }
    // Compute secondary prescale value to get closest SPI clock freq to that 
    // specified
    secondary = (uint16_t)(0.5+FCY/freq);
    secondary = (8-secondary)<<2;   // Map secondary prescale bits for SPIxCON1
    // Configure the SPI module
    //   set SPI module to 8-bit master mode
    //   set SMP = 0, CKE = 1, and CKP = 0
    //   set SPRE and PPRE bits to get the closest SPI clock freq to that 
    //   specified
    *(self->SPIxCON1) = 0x0120|primary|secondary;
    *(self->SPIxCON2) = 0;
    // Enable the SPI module and clear status flags
    *(self->SPIxSTAT) = 0x8000;
}

void spi_close(_SPI *self) {
    *(self->SPIxSTAT) = 0;
    *(self->SPIxCON1) = 0;
    *(self->SPIxCON2) = 0;
    if (self->MISO) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->MISOrpinr) |= 0x3F<<(self->MISOrpshift);
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->MISO->owner = NULL;
        pin_digitalIn(self->MISO);
        self->MISO = NULL;
    }
    if (self->MOSI) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->MOSI->rpor) &= ~(0x3F<<(self->MOSI->rpshift));
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->MOSI->owner = NULL;
        pin_digitalOut(self->MOSI);
        pin_set(self->MOSI);
        self->MOSI = NULL;
    }
    if (self->SCK) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->SCK->rpor) &= ~(0x3F<<(self->SCK->rpshift));
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->SCK->owner = NULL;
        pin_digitalOut(self->SCK);
        pin_clear(self->SCK);
        self->SCK = NULL;
    }
}

uint8_t spi_transfer(_SPI *self, uint8_t val) {
    *(self->SPIxBUF) = (uint16_t)val;
    while (bitread(self->SPIxSTAT, 0)==0) {}
    return (uint8_t)(*(self->SPIxBUF));
}
