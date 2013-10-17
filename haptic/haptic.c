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

// const float PIN_CONVERSION_FACTOR = 65472;
const float MAX_VOLTAGE = 3;
const float MIN_WIDTH = 5.5E-4;
const float MAX_WIDTH = 2.3E-3;
const float FREQ = 40e3;
// const uint16_t PEAK_DETECT_DIFF = 21824;
const uint16_t ZERO_DUTY = 0;
const uint16_t HALF_DUTY = 32768;
const float INTERVAL = 0.02; //for servos
const uint16_t PULSE_WIDTH = 1 << 8;
const uint16_t ECHO_TIME = 0b1001 << 7;
const float PULSE_FREQUENCY = 0.07; //how often we send a pulse

uint16_t prev_signal_value;
uint16_t current_signal_value;

uint16_t send_pulse = 0;

uint16_t get_distance = 0;

uint16_t signal_send_time;
uint16_t peak_detect_time;

uint16_t time_of_flight;

uint16_t pos;
uint16_t val1, val2;

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

    //setup the signal input pin
    pin_digitalIn(&D[4]);

    val1 = 0;
    val2 = 0;
    pos = 0; //16 bit int with binary point in front of the MSB

    led_on(&led2);
    timer_setPeriod(&timer2, PULSE_FREQUENCY); //how often we send a pulse
    timer_start(&timer2);
    timer_setPeriod(&timer3, 0.5); //heartbeat
    timer_start(&timer3);

    oc_servo(&oc1,&D[0],&timer4, INTERVAL,MIN_WIDTH, MAX_WIDTH, pos);
    oc_servo(&oc2,&D[2],&timer5, INTERVAL,MIN_WIDTH, MAX_WIDTH, pos);
    oc_pwm(&oc3,&D[3],NULL,FREQ,ZERO_DUTY);

    printf("Good morning\n");

    InitUSB();                              // initialize the USB registers and serial interface engine
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        ServiceUSB();
        pin_write(&D[0],val1);
        pin_write(&D[2],val2); 
        //adapted from Patrick and Charlie's approach
        if (!send_pulse && timer_read(&timer2) < PULSE_WIDTH){
            send_pulse = 1;
            pin_write(&D[3],HALF_DUTY);     
            get_distance = 1;
        } else if (send_pulse && timer_read(&timer2) >= PULSE_WIDTH) {
            send_pulse = 0;
            pin_write(&D[3],ZERO_DUTY); 
        }

        if (timer_read(&timer2) >= ECHO_TIME)
        {
            if (pin_read(&D[4]) && get_distance)
            {
                printf("%d\n", timer_read(&timer2));
                get_distance = 0;
            }
        }
       if (timer_flag(&timer3)) {
            //show a heartbeat and a status message
            timer_lower(&timer3);
            led_toggle(&led1);
        }

    }
}


