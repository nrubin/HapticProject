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
#include "pin.h"
#include "uart.h"

_UART uart1, uart2, uart3, uart4;
_UART *_stdout, *_stderr;
_PIN AJTX, AJRX;

void __putc_nobuffer(_UART *self, uint8_t ch) {
    while (bitread(self->UxSTA, 9)==1) {}   // Wait until TX buffer is not full
    *(self->UxTXREG) = (uint16_t)ch;
}

uint8_t __getc_nobuffer(_UART *self) {
    while (bitread(self->UxSTA, 0)==0) {} // Wait until RX buffer is not empty
    return (uint8_t)(*(self->UxRXREG));
}

void __putc_buffer(_UART *self, uint8_t ch) {
    while (self->TXbuffer.count==self->TXbuffer.length) {}  // Wait until TX 
                                                            // buffer is not 
                                                            // full
    self->TXbuffer.data[self->TXbuffer.tail] = ch;
    self->TXbuffer.tail = (self->TXbuffer.tail+1)%(self->TXbuffer.length);
    self->TXbuffer.count++;
    if (self->TXbuffer.count>=self->TXthreshold)    // If TX buffer is full 
        bitset(self->UxSTA, 10);                    // enough, enable data
                                                    // transmission
}

void __serviceTxInterrupt(_UART *self) {
    uint8_t ch;

    bitclear(self->IFSy, self->UxTXIF); // Lower TX interrupt flag
    if (self->TXbuffer.count==0)        // If nothing left in TX buffer, 
        bitclear(self->UxSTA, 10);      // disable data transmission
    while ((bitread(self->UxSTA, 9)==0) && (self->TXbuffer.count!=0)) {
        ch = self->TXbuffer.data[self->TXbuffer.head];
        self->TXbuffer.head = (self->TXbuffer.head+1)%(self->TXbuffer.length);
        self->TXbuffer.count--;
        *(self->UxTXREG) = (uint16_t)ch;
    }
}

uint8_t __getc_buffer(_UART *self) {
    uint8_t ch;

    while (self->RXbuffer.count==0) {}  // Wait until RX buffer is not empty
    ch = self->RXbuffer.data[self->RXbuffer.head];
    self->RXbuffer.head = (self->RXbuffer.head+1)%(self->RXbuffer.length);
    self->RXbuffer.count--;
    return ch;
}

void __serviceRxInterrupt(_UART *self) {
    bitclear(self->IFSy, self->UxRXIF); // Lower RX interrupt flag
    while ((bitread(self->UxSTA, 0)==1) && 
           (self->RXbuffer.count!=self->RXbuffer.length)) {
        self->RXbuffer.data[self->RXbuffer.tail] = (uint8_t)(*(self->UxRXREG));
        self->RXbuffer.tail = (self->RXbuffer.tail+1)%(self->RXbuffer.length);
        self->RXbuffer.count++;
    }
}

void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void) {
    __serviceTxInterrupt(&uart1);
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void) {
    __serviceRxInterrupt(&uart1);
}

void __attribute__((interrupt, auto_psv)) _U2TXInterrupt(void) {
    __serviceTxInterrupt(&uart2);
}

void __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void) {
    __serviceRxInterrupt(&uart2);
}

void __attribute__((interrupt, auto_psv)) _U3TXInterrupt(void) {
    __serviceTxInterrupt(&uart3);
}

void __attribute__((interrupt, auto_psv)) _U3RXInterrupt(void) {
    __serviceRxInterrupt(&uart3);
}

void __attribute__((interrupt, auto_psv)) _U4TXInterrupt(void) {
    __serviceTxInterrupt(&uart4);
}

void __attribute__((interrupt, auto_psv)) _U4RXInterrupt(void) {
    __serviceRxInterrupt(&uart4);
}

int16_t write(int16_t handle, void *buffer, uint16_t len) {
    int16_t i;

    switch (handle) {
        case 0:
        case 1:
            for (i = 0; i<len; i++)
                uart_putc(_stdout, *(uint8_t *)buffer++);
            break;
        case 2:
            for (i = 0; i<len; i++)
                uart_putc(_stderr, *(uint8_t *)buffer++);
            break;
        default:
            break;
    }
    return len;
}

void init_uart(void) {
    init_pin();

    pin_init(&AJTX, (uint16_t *)&PORTG, (uint16_t *)&TRISG, 
             (uint16_t *)NULL, 6, -1, 8, 21, (uint16_t *)&RPOR10);
    pin_init(&AJRX, (uint16_t *)&PORTG, (uint16_t *)&TRISG, 
             (uint16_t *)NULL, 7, -1, 0, 26, (uint16_t *)&RPOR13);

    uart_init(&uart1, (uint16_t *)&U1MODE, (uint16_t *)&U1STA, 
              (uint16_t *)&U1TXREG, (uint16_t *)&U1RXREG, 
              (uint16_t *)&U1BRG, (uint16_t *)&IFS0, 
              (uint16_t *)&IEC0, 12, 11, (uint16_t *)&RPINR18, 
              (uint16_t *)&RPINR18, 0, 8, 3, 4);
    uart_init(&uart2, (uint16_t *)&U2MODE, (uint16_t *)&U2STA, 
              (uint16_t *)&U2TXREG, (uint16_t *)&U2RXREG, 
              (uint16_t *)&U2BRG, (uint16_t *)&IFS1, 
              (uint16_t *)&IEC1, 15, 14, (uint16_t *)&RPINR19, 
              (uint16_t *)&RPINR19, 0, 8, 5, 6);
    uart_init(&uart3, (uint16_t *)&U3MODE, (uint16_t *)&U3STA, 
              (uint16_t *)&U3TXREG, (uint16_t *)&U3RXREG, 
              (uint16_t *)&U3BRG, (uint16_t *)&IFS5, 
              (uint16_t *)&IEC5, 3, 2, (uint16_t *)&RPINR17, 
              (uint16_t *)&RPINR21, 8, 8, 28, 29);
    uart_init(&uart4, (uint16_t *)&U4MODE, (uint16_t *)&U4STA, 
              (uint16_t *)&U4TXREG, (uint16_t *)&U4RXREG, 
              (uint16_t *)&U4BRG, (uint16_t *)&IFS5, 
              (uint16_t *)&IEC5, 9, 8, (uint16_t *)&RPINR27, 
              (uint16_t *)&RPINR27, 0, 8, 30, 31);

    uart_open(&uart1, &AJTX, &AJRX, NULL, NULL, 19200., 'N', 1, 
              0, NULL, 0, NULL, 0);

    _stdout = &uart1;
    _stderr = &uart1;
}

void uart_init(_UART *self, uint16_t *UxMODE, uint16_t *UxSTA, 
               uint16_t *UxTXREG, uint16_t *UxRXREG, 
               uint16_t *UxBRG, uint16_t *IFSy, uint16_t *IECy, 
               uint8_t UxTXIF, uint8_t UxRXIF, 
               uint16_t *RXrpinr, uint16_t *CTSrpinr, 
               uint8_t RXrpshift, uint8_t CTSrpshift, 
               int16_t TXrpnum, int16_t RTSrpnum) {
    self->UxMODE = UxMODE;
    self->UxSTA = UxSTA;
    self->UxTXREG = UxTXREG;
    self->UxRXREG = UxRXREG;
    self->UxBRG = UxBRG;
    self->IFSy = IFSy;
    self->IECy = IECy;
    self->UxTXIF = UxTXIF;
    self->UxRXIF = UxRXIF;
    self->RXrpinr = RXrpinr;
    self->CTSrpinr = CTSrpinr;
    self->RXrpshift = RXrpshift;
    self->CTSrpshift = CTSrpshift;
    self->TXrpnum = TXrpnum;
    self->RTSrpnum = RTSrpnum;
    self->TX = NULL;
    self->RX = NULL;
    self->RTS = NULL;
    self->CTS = NULL;
    self->TXbuffer.data = NULL;
    self->TXbuffer.length = 0;
    self->TXbuffer.head = 0;
    self->TXbuffer.tail = 0;
    self->TXbuffer.count = 0;
    self->RXbuffer.data = NULL;
    self->RXbuffer.length = 0;
    self->RXbuffer.head = 0;
    self->RXbuffer.tail = 0;
    self->RXbuffer.count = 0;
    self->TXthreshold = 1;
    self->putc = NULL;
    self->getc = NULL;
}

void uart_open(_UART *self, _PIN *TX, _PIN *RX, _PIN *RTS, _PIN *CTS, 
               float baudrate, int8_t parity, int16_t stopbits, 
               uint16_t TXthreshold, uint8_t *TXbuffer, uint16_t TXbufferlen, 
               uint8_t *RXbuffer, uint16_t RXbufferlen) {
    *(self->UxMODE) = 0;    // Disable UART module, set UEN<1:0> = 00 
                            // (i.e., TX and RX pins are enabled and used),
                            // set BRGH = 0 (i.e., standard speed baud mode for 
                            // rate generator), set PDSEL<1:0> = 00 (i.e.,
                            // 8-bit data and no parity), set STSEL = 0 (i.e.,
                            // one data bit)
    if ((TX->rpnum==-1) || (RX->rpnum==-1))
        return; // At least one of the pins specified for TX and RX is not an 
                // RP pin
    if ((TX->owner==NULL) && (RX->owner==NULL)) {
        // Both pins specified for TX and RX are available, so configure as 
        // specified
        pin_digitalOut(TX);
        pin_set(TX);
        pin_digitalIn(RX);
        self->TX = TX;
        TX->owner = (void *)self;
        TX->write = NULL;
        TX->read = NULL;
        self->RX = RX;
        RX->owner = (void *)self;
        RX->write = NULL;
        RX->read = NULL;
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(TX->rpor) &= ~(0x3F<<(TX->rpshift));
        *(TX->rpor) |= (self->TXrpnum)<<(TX->rpshift);
        *(self->RXrpinr) &= ~(0x3F<<(self->RXrpshift));
        *(self->RXrpinr) |= (RX->rpnum)<<(self->RXrpshift);
        __builtin_write_OSCCONL(OSCCON|0x40);
    } else if ((self->TX!=TX) || (self->RX!=RX)) {
        return; // At least one of the pins specified for TX and RX do not 
                // match the previous assignment
    }
    if ((RTS!=NULL) && (CTS!=NULL)) {
        if ((RTS->rpnum==-1) || (CTS->rpnum==-1))
            return;     // At least one of the pins specified for RTS and CTS 
                        // is not an RP pin
        if ((RTS->owner==NULL) && (CTS->owner==NULL)) {
            // Both pins specified for RTS and CTS are available, so configure 
            // as specified
            pin_digitalOut(RTS);
            pin_set(RTS);
            pin_digitalIn(CTS);
            self->RTS = RTS;
            RTS->owner = (void *)self;
            RTS->write = NULL;
            RTS->read = NULL;
            self->CTS = CTS;
            CTS->owner = (void *)self;
            CTS->write = NULL;
            CTS->read = NULL;
            __builtin_write_OSCCONL(OSCCON&0xBF);
            *(RTS->rpor) &= ~(0x3F<<(RTS->rpshift));
            *(RTS->rpor) |= (self->RTSrpnum)<<(RTS->rpshift);
            *(self->CTSrpinr) &= ~(0x3F<<(self->CTSrpshift));
            *(self->CTSrpinr) |= (CTS->rpnum)<<(self->CTSrpshift);
            __builtin_write_OSCCONL(OSCCON|0x40);
        } else if ((self->RTS!=RTS) || (self->CTS!=CTS)) {
            return;     // At least one of the pins specified for RTS and CTS 
                        // do not match the previous assignment
        }
        *(self->UxMODE) = 0x0200;   // Set UEN<1:0> = 10, indicating TX, RX,
                                    // RTS, and CTS are enabled and used
    } else if ((RTS!=NULL) || (CTS!=NULL)) {
        return; // A pin was specified for RTS or CTS, but not both
    }
    // Clip baudrate to be in allowable range of values
    if (baudrate>(FCY/4.))
        baudrate = FCY/4.;
    if (baudrate<(FCY/(16.*65536.)))
        baudrate = FCY/(16.*65536.);
    // Select BRGH value and compute BRG value
    if (baudrate<=(FCY/(4.*65536.))) {
        *(self->UxBRG) = (uint16_t)(0.5+(FCY/16.)/baudrate)-1;
    } else {
        bitset(self->UxMODE, 3);    // Set BRGH = 1
        *(self->UxBRG) = (uint16_t)(0.5+(FCY/4.)/baudrate)-1;
    }
    // Set parity as specified
    if ((parity=='E') || (parity=='e')) {
        bitset(self->UxMODE, 1);    // Set PDSEL<1:0> = 01, indicating 8-bit
                                    // data and even parity
    } else if ((parity=='O') || (parity=='o')) {
        bitset(self->UxMODE, 2);    // Set PDSEL<1:0> = 10, indicating 8-bit
                                    // data and odd parity
    } else if ((parity!='N') && (parity!='n')) {
        return; // Illegitimate parity setting specified
    }
    // Set stopbits as specified
    if (stopbits==2) {
        bitset(self->UxMODE, 0);    // Set STSEL = 1
    } else if (stopbits!=1) {
        return; // Illegitimate stopbit setting specified
    }
    // Set up transmit buffer and interrupt as specified
    if (TXbuffer==NULL) {
        self->putc = __putc_nobuffer;
    } else {
        bitset(self->UxSTA, 13);    // Set UTXISEL<1:0> = 01, TX interrupt when
                                    // all transmit operations are done
        self->TXbuffer.data = TXbuffer;
        self->TXbuffer.length = TXbufferlen;
        self->TXbuffer.head = 0;
        self->TXbuffer.tail = 0;
        self->TXbuffer.count = 0;
        self->TXthreshold = TXthreshold;
        self->putc = __putc_buffer;
        bitclear(self->IFSy, self->UxTXIF); // Lower TX interrupt flag
        bitset(self->IECy, self->UxTXIF);   // Enable TX interrupt
    }
    // Set up receive buffer and interrupt as specified
    if (RXbuffer==NULL) {
        self->getc = __getc_nobuffer;
    } else {
        self->RXbuffer.data = RXbuffer;
        self->RXbuffer.length = RXbufferlen;
        self->RXbuffer.head = 0;
        self->RXbuffer.tail = 0;
        self->RXbuffer.count = 0;
        self->getc = __getc_buffer;
        bitclear(self->IFSy, self->UxRXIF); // Lower RX interrupt flag
        bitset(self->IECy, self->UxRXIF);   // Enable RX interrupt
    }
    bitset(self->UxMODE, 15);   // Enable UART module
    bitset(self->UxSTA, 10);    // Enable data transmission
}

void uart_close(_UART *self) {
    bitclear(self->UxSTA, 10);  // Disable data transmission
    *(self->UxMODE) = 0;        // Disable UART module
    bitclear(self->IECy, self->UxTXIF); // Disable TX interrupt
    bitclear(self->IECy, self->UxRXIF); // Disable RX interrupt
    self->putc = NULL;
    self->getc = NULL;
    if (self->TX) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->TX->rpor) &= ~(0x3F<<(self->TX->rpshift));
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->TX->owner = NULL;
        pin_digitalOut(self->TX);
        pin_set(self->TX);
        self->TX = NULL;
    }
    if (self->RX) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->RXrpinr) |= 0x3F<<(self->RXrpshift);
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->RX->owner = NULL;
        pin_digitalIn(self->RX);
        self->RX = NULL;
    }
    if (self->RTS) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->RTS->rpor) &= ~(0x3F<<(self->RTS->rpshift));
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->RTS->owner = NULL;
        pin_digitalOut(self->RTS);
        pin_set(self->RTS);
        self->RTS = NULL;
    }
    if (self->CTS) {
        __builtin_write_OSCCONL(OSCCON&0xBF);
        *(self->CTSrpinr) |= 0x3F<<(self->CTSrpshift);
        __builtin_write_OSCCONL(OSCCON|0x40);
        self->CTS->owner = NULL;
        pin_digitalIn(self->CTS);
        self->CTS = NULL;
    }
}

void uart_putc(_UART *self, uint8_t ch) {
    if (self->putc)
        self->putc(self, ch);
}

uint8_t uart_getc(_UART *self) {
    if (self->getc)
        return self->getc(self);
    else
        return 0xFF;
}

void uart_flushTxBuffer(_UART *self) {
    if (bitread(self->UxSTA, 10)==0)    // If transmission is disabled,
        bitset(self->UxSTA, 10);        //   enable data transmission
}

void uart_puts(_UART *self, uint8_t *str) {
    while (*str)
        uart_putc(self, *str++);
    uart_flushTxBuffer(self);
}

void uart_gets(_UART *self, uint8_t *str, uint16_t len) {
    uint8_t *start;
    uint16_t left;

    if (len==0)
        return;

    if (len==1) {
        *str = '\0';
        return;
    }

    uart_putc(self, 0x1B);                  // Save current cursor position
    uart_putc(self, '7');
    uart_flushTxBuffer(self);
    start = str;
    left = len;
    while (1) {
        *str = uart_getc(self);             // Get a character
        if (*str=='\r')                     // If character is return,
            break;                          //   end the loop.
        if (*str==0x1B) {                   // If character is escape,
            uart_putc(self, 0x1B);          //   restore cursor position,
            uart_putc(self, '8');
            uart_putc(self, 0x1B);          //   clear to end of line, and
            uart_putc(self, '[');
            uart_putc(self, 'K');
            uart_flushTxBuffer(self);
            str = start;                    //   start over at the beginning.
            left = len;
            continue;
        }
        if ((*str=='\b') ||                 // If character is backspace
            (*str==0x7F)) {                 //   or delete, 
            if (str>start) {                //   and we are not at the start, 
                uart_putc(self, '\b');      //   erase the last character and
                uart_putc(self, ' ');
                uart_putc(self, '\b');
                uart_flushTxBuffer(self);
                str--;                      //   back up the pointer,
                left++;
            } else {                        //   otherwise
                uart_putc(self, '\a');      //   send alert/bell character.
                uart_flushTxBuffer(self);
            }
            continue;
        }
        if (left==1) {                      // If string buffer is full,
            uart_putc(self, '\a');          //   send alert/bell character
            uart_flushTxBuffer(self);
            continue;
        }
        if ((*str>=32) && (*str<127)) {     // If character is printable,
            uart_putc(self, *str);          //   echo the received character
            uart_flushTxBuffer(self);
            str++;                          //   and advance the pointer.
            left--;
        }
    }
    *str = '\0';                            // Terminarte the string with null
    uart_putc(self, '\n');                  // Send newline and
    uart_putc(self, '\r');                  //   carriage return
    uart_flushTxBuffer(self);
}

