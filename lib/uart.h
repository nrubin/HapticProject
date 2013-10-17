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
#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
#include "pin.h"

void init_uart(void);

typedef struct _RINGBUFFER {
    uint8_t *data;
    uint16_t length;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} _RINGBUFFER;

typedef struct _UART {
    uint16_t *UxMODE;
    uint16_t *UxSTA;
    uint16_t *UxTXREG;
    uint16_t *UxRXREG;
    uint16_t *UxBRG;
    uint16_t *IFSy;
    uint16_t *IECy;
    uint8_t UxTXIF;
    uint8_t UxRXIF;
    uint16_t *RXrpinr;
    uint16_t *CTSrpinr;
    uint8_t RXrpshift;
    uint8_t CTSrpshift;
    int16_t TXrpnum;
    int16_t RTSrpnum;
    _PIN *TX;
    _PIN *RX;
    _PIN *RTS;
    _PIN *CTS;
    _RINGBUFFER TXbuffer;
    _RINGBUFFER RXbuffer;
    uint16_t TXthreshold;
    void (*putc)(struct _UART *self, uint8_t ch);
    uint8_t (*getc)(struct _UART *self);
} _UART;

extern _UART uart1, uart2, uart3, uart4;
extern _UART *_stdout, *_stderr;
extern _PIN AJTX, AJRX;

void uart_init(_UART *self, uint16_t *UxMODE, uint16_t *UxSTA, 
               uint16_t *UxTXREG, uint16_t *UxRXREG, 
               uint16_t *UxBRG, uint16_t *IFSy, uint16_t *IECy, 
               uint8_t UxTXIF, uint8_t UxRXIF, 
               uint16_t *RXrpinr, uint16_t *CTSrpinr, 
               uint8_t RXrpshift, uint8_t CTSrpshift, 
               int16_t TXrpnum, int16_t RTSrpnum);
void uart_open(_UART *self, _PIN *TX, _PIN *RX, _PIN *RTS, _PIN *CTS, 
               float baudrate, int8_t parity, int16_t stopbits, 
               uint16_t TXthreshold, uint8_t *TXbuffer, uint16_t TXbufferlen, 
               uint8_t *RXbuffer, uint16_t RXbufferlen);
void uart_close(_UART *self);
void uart_putc(_UART *self, uint8_t ch);
uint8_t uart_getc(_UART *self);
void uart_flushTxBuffer(_UART *self);
void uart_puts(_UART *self, uint8_t *str);
void uart_gets(_UART *self, uint8_t *str, uint16_t len);

#endif

