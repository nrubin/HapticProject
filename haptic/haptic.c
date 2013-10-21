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
#define SET_VALS    1   // Vendor request that receives 2 unenesigned integer values
#define GET_VALS    2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS  3   // Vendor request that prints 2 unsigned integer values 

const float FREQ = 25e3; //assuming this divides the processor frequency, the min freq we can do is 244.1 Hz.
const uint16_t ZERO_DUTY = 0;
const uint16_t HALF_DUTY = 32768; //6 volts
const uint16_t QUARTER_DUTY = 16384; //6 volts
const uint16_t THREE_QUARTER_DUTY = 49152; //6 volts
const uint16_t FULL_DUTY = 65535; //6 volts

// void __attribute__((interrupt)) _CNInterrupt(void); 
void __attribute__((interrupt)) _OC1Interrupt(void); 

uint16_t val1, val2;
uint16_t slow = 0;
// CNEN1 =  1<<14;

uint16_t count;
uint16_t feedback_current;
// CNEN1bits

// void SetupTach(void){
// // CNEN2 = 0b0000000000000010;  //looks like d[0] is RP10 AKA CN17
// // IFS1 = 0; //Bit 4 of IFS1 is CNIF, the interrupt flag we want to raise/lower
// // _IEC1 = 1;
// }

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

void init_motor(void){
    pin_digitalIn(&D[0]); //tach input
    pin_digitalOut(&D[2]); //D2-bar
    pin_digitalOut(&D[3]); //D1
    pin_digitalOut(&D[4]); //ENA
    pin_digitalOut(&D[7]); //SLEW
    pin_digitalOut(&D[8]); //INV
    pin_analogIn(&A[0]); //current sensor?
    pin_analogIn(&A[1]); //Vemf sensor
    pin_analogIn(&A[2]); //0.24% of active high side current

    pin_write(&D[2],1); //no tri-stating!
    pin_write(&D[3],0); //no tri-stating!
    pin_write(&D[4],1); //Enable the system
    pin_write(&D[7],0); //low slew rate
    pin_write(&D[8],0); //don't invert the inputs!    
}

// void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
//     printf("interrupt!\n");
//     pin_read(&D[0]);
//     count = 1;
//     IFS1bits.CNIF = 0;
// }

void __attribute__((interrupt, auto_psv)) _OC1Interrupt(void) {
    IFS0bits.OC1IF = 0;
    // printf("count: %d \n", count);
    count++;
    val2 = count;
    // led_toggle(&led3);
}



int16_t main(void) {
    // SetupTach();
    init_pin();
    init_clock();
    init_uart();
    init_ui();
    init_timer();
    init_oc();
    // init_motor();
    InitUSB(); // initialize the USB registers and serial interface engine    
    count = 0;
    // CNEN1bits.CN14IE = 1;  //sets the second bit of CNEN1, CN1IE (change notif 1 interrupt enable)
    // IFS1bits.CNIF = 0; //make sure the interrupt flag is set low
    // IEC1bits.CNIE = 1; //make sure all change notif inputs are on
    // CNPD1bits.CN14PDE = 1; //enable the pulldown resistor on CN14
    IEC0bits.OC1IE = 1;
    IFS0bits.OC1IF = 0;
    led_off(&led3);
    led_on(&led2);
    timer_setPeriod(&timer3, 0.5);
    timer_start(&timer3);
    timer_setPeriod(&timer1, 1);
    timer_start(&timer1);
    // printf("count: %d \n", count);

    // oc_pwm(&oc3,&D[5],NULL,FREQ,FULL_DUTY);
    oc_pwm(&oc1,&D[5],NULL,FREQ,HALF_DUTY);
    // oc_pwm(&oc3,&D[6],NULL,FREQ,QUARTER_DUTY);
    
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        // pin_write(&D[5],val1); //11000 seems max, 32000 seems min @ 40e3
        ServiceUSB();
         if (timer_flag(&timer3)) {
            //show a heartbeat
            timer_lower(&timer3);
            led_toggle(&led1);
            // feedback_current = pin_read(&A[0]);
            // val2 = feedback_current;
            // printf("count: %d \n", count);
        }   
         if (timer_flag(&timer1)) {
            //show a heartbeat
            timer_lower(&timer1);
            count = 0;
            // feedback_current = pin_read(&A[0]);
            // val2 = feedback_current;
            // printf("count: %d \n", count);
        }  
    }
}


