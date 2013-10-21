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

#define INV &D[8]
#define ENA &D[4]
#define IN1 &D[5]
#define IN2 &D[6]
#define D1 &D[3]
#define D2n &D[2]
#define PWM_TIMER &timer2

const float FREQ = 10; //assuming this divides the processor frequency, the min freq we can do is 244.1 Hz.
const uint16_t ZERO_DUTY = 0;
const uint16_t HALF_DUTY = 32768; //6 volts
const uint16_t QUARTER_DUTY = 16384; //6 volts
const uint16_t THREE_QUARTER_DUTY = 49152; //6 volts
const uint16_t FULL_DUTY = 65535; //6 volts

//declare interrupts
void __attribute__((interrupt)) _CNInterrupt(void); 
void __attribute__((interrupt)) _OC3Interrupt(void); 
void __attribute__((interrupt)) _OC4Interrupt(void); 

uint16_t val1, val2;
uint16_t rev_count;
uint16_t direction;
uint16_t feedback_current;

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


void toggle_direction(void){
    direction = !direction;
    if (direction) {
        pin_write(IN1,0);
        pin_write(IN2,1);
    } else{
        pin_write(IN1,1);
        pin_write(IN2,0);
    }
    // pin_write(INV,direction);
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    IFS1bits.CNIF = 0;
    rev_count++;
    // printf("interrupt!\n");
    pin_read(&D[0]);
    // count = 1;
}

void __attribute__((interrupt, auto_psv)) _OC3Interrupt(void) {
    IFS1bits.OC3IF = 0;
    // printf("count: %d \n", count);
    // count++;
    // val2 = count;
    // led_toggle(&led3);
}

void __attribute__((interrupt, auto_psv)) _OC4Interrupt(void) {
    IFS1bits.OC4IF = 0;
    // printf("count: %d \n", count);
    // count++;
    // val2 = count;
    // led_toggle(&led3);
}

void init(void){
    init_pin();
    init_clock();
    init_uart();
    init_ui();
    init_timer();
    init_oc();
    init_motor();
    InitUSB(); // initialize the USB registers and serial interface engine    
    init_interrupts();
}

void init_interrupts(void){
    CNEN1bits.CN14IE = 1;  //sets the second bit of CNEN1, CN1IE (change notif 1 interrupt enable)
    IFS1bits.CNIF = 0; //make sure the interrupt flag is set low
    IEC1bits.CNIE = 1; //make sure all change notif inputs are on

    // CNPD1bits.CN14PDE = 1; //enable the pulldown resistor on CN14

    // enable OC3 and OC4 interrupts
    IEC1bits.OC3IE = 1; 
    IEC1bits.OC4IE = 1;
    //Set OC interrupt flags low
    IFS1bits.OC3IF = 0;
    IFS1bits.OC4IF = 0;
}

void init_motor(void){
    pin_digitalIn(&D[0]); //tach input
    // pin_digitalOut(D2n); //D2-bar
    pin_digitalOut(IN1); //D2-bar
    pin_digitalOut(IN2); //D2-bar
    pin_digitalOut(D1); //D1
    pin_digitalOut(ENA); //ENA
    pin_digitalOut(&D[7]); //SLEW
    pin_digitalOut(INV); //INV
    pin_analogIn(&A[0]); //current sensor?
    pin_analogIn(&A[1]); //Vemf sensor
    pin_analogIn(&A[2]); //0.24% of active high side current
    pin_write(IN1,1);
    pin_write(IN2,0);
    pin_write(D1,0); //no tri-stating!
    // pin_write(D2n,0); // PWM on this
    pin_write(ENA,1); //Enable the system
    pin_write(&D[7],0); //low slew rate
    pin_write(INV,0); //don't invert the inputs!    
}

int16_t main(void) {
    init();
    rev_count = 0;
    direction = 0;
    led_off(&led3);
    led_on(&led2);
    timer_setPeriod(&timer3, 0.5);
    timer_start(&timer3);
    timer_setPeriod(&timer1, 1);
    timer_start(&timer1);
    printf("Good morning!\n");

    oc_pwm(&oc3,D2n,PWM_TIMER,FREQ,0);
    
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        pin_write(D2n,val1); //11000 seems max, 32000 seems min @ 40e3
        ServiceUSB();
         if (timer_flag(&timer3)) {
            //show a heartbeat
            timer_lower(&timer3);
            led_toggle(&led1);
            // toggle_direction();
            // feedback_current = pin_read(&A[0]);
            // val2 = feedback_current;
            // printf("count: %d \n", count);
        }     
        if (timer_flag(&timer1)) {
            // toggle_direction();
            printf("count = %d\n",rev_count);
            timer_lower(&timer1);
            rev_count = 0;
        }   
    }
}


