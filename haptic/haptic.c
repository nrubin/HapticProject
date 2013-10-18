#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "usb.h"
#include "pin.h"
#include "uart.h"
#include "oc.h"
#include "ui.h"
#include "timer.h"
#include <stdio.h>

#define HELLO       0   // Vendor request that prints "Hello World!"
#define SET_VALS    1   // Vendor request that receives 2 unsigned integer values
#define GET_VALS    2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS  3   // Vendor request that prints 2 unsigned integer values 

const float FREQ = 40e3;
const uint16_t ZERO_DUTY = 0;
const uint16_t HALF_DUTY = 32768; //6 volts
const uint16_t QUARTER_DUTY = 16384; //6 volts
const uint16_t THREE_QUARTER_DUTY = 49152; //6 volts

uint16_t val1, val2;
uint16_t slow = 0;



void VendorRequests(void) {
    WORD temp;

    switch (USB_setup.bRequest) {
        case HELLO:
            printf("Hello World!\n");
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_VALS:
            val1 = USB_setup.wValue.w;
            val2 = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_VALS:
            temp.w = val1;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.w = val2;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 4
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;            
        case PRINT_VALS:
            printf("val1 = %u, val2 = %u\n", val1, val2);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        default:
            USB_error_flags |= 0x01;    // set Request Error Flag
    }
}

void VendorRequestsIn(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void VendorRequestsOut(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

int16_t main(void) {
    init_pin();
    init_clock();
    init_uart();
    init_ui();
    init_timer();
    init_oc();

    pin_digitalOut(&D[2]); //D2
    pin_digitalOut(&D[3]); //D1
    pin_digitalOut(&D[4]); //ENA
    // pin_digitalOut(&D[5]); //IN2
    // pin_digitalOut(&D[6]); //IN1
    pin_digitalOut(&D[7]); //SLEW
    pin_digitalOut(&D[8]); //INV

    pin_write(&D[2],1); //no tri-stating!
    pin_write(&D[3],0); //no tri-stating!
    pin_write(&D[4],1); //Enable the system
    pin_write(&D[7],0); //low slew rat
    pin_write(&D[8],0); //don't invert the inputs!

    led_on(&led2);
    timer_setPeriod(&timer3, 0.5); //heartbeat
    timer_start(&timer3);
    timer_setPeriod(&timer3, 2); //heartbeat
    timer_start(&timer3);

    oc_pwm(&oc3,&D[6],NULL,FREQ,QUARTER_DUTY);
    // oc_pwm(&oc3,&D[6],NULL,FREQ,QUARTER_DUTY);
    while(1){

         if (timer_flag(&timer3)) {
            //show a heartbeat and a status message
            timer_lower(&timer3);
            if(slow){
                pin_write(&D[6],HALF_DUTY);   
            } else {
                pin_write(&D[6],QUARTER_DUTY);
            }
            led_toggle(&led1);
            slow = !slow;
        }   
        // if (!sw_read(&sw2))
        // {
        //     slow = !slow;
        //     // pin_write(&D[3],slow); //Enable the system
        //     // pin_write(&D[2],slow); //Enable the system
        //     led_toggle(&led3);
        // }
    }
}


