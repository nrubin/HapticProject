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

#define SET_VELOCITY    0   // Vendor request that receives 2 unenesigned integer values
#define GET_VALS    1   // Vendor request that returns 2 unsigned integer values

#define INV &D[8]
#define ENA &D[4]
#define IN1 &D[5]
#define IN2 &D[6]
#define D1 &D[3]
#define D2n &D[2]
#define PWM_TIMER &timer2
#define FB_PIN &A[2]
#define CURRENT_PIN &A[0]
#define VEMF_PIN &A[1]
#define REV_PIN &D[0]
#define HB_TIMER &timer3
// #define FB &A[2]

const float FREQ = 250; //assuming this divides the processor frequency, the min freq we can do is 244.1 Hz.
const uint16_t ZERO_DUTY = 0;
const uint16_t HALF_DUTY = 32768; //6 volts
const uint16_t QUARTER_DUTY = 16384; //6 volts
const uint16_t THREE_QUARTER_DUTY = 49152; //6 volts
const uint16_t FULL_DUTY = 65535; //6 volts

//declare interrupts
void __attribute__((interrupt)) _CNInterrupt(void); 
void __attribute__((interrupt)) _OC3Interrupt(void); 

uint16_t val1, val2;

uint16_t REV; // 1 hundredth of a cycle
uint16_t FB; //current feedback
uint16_t DUTY; //current duty cycle
uint16_t REQUESTED_DIRECTION; //direction motor should go
uint16_t SENSED_DIRECTION; //direction motor is going
uint16_t VEMF; //vemf

void VendorRequests(void) {
    WORD temp;

    switch (USB_setup.bRequest) {
        case SET_VELOCITY:
            DUTY = USB_setup.wValue.w;
            REQUESTED_DIRECTION = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_VALS:
            temp.w = REV;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.w = FB;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            temp.w = SENSED_DIRECTION;
            BD[EP0IN].address[4] = temp.b[0];
            BD[EP0IN].address[5] = temp.b[1];
            temp.w = VEMF;
            BD[EP0IN].address[6] = temp.b[0];
            BD[EP0IN].address[7] = temp.b[1];
            BD[EP0IN].bytecount = 8;    // set EP0 IN byte count to 4
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


void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    IFS1bits.CNIF = 0; //lower the flag
    if (SENSED_DIRECTION) //count in the right direction
    {
        REV++;
    } else {
        REV--;
    }
    pin_read(REV_PIN); //clear the pin
}

void __attribute__((interrupt, auto_psv)) _OC3Interrupt(void) {
    IFS1bits.OC3IF = 0;
}

void __attribute__((interrupt, auto_psv)) _OC4Interrupt(void) {
    IFS1bits.OC4IF = 0;
}

void init_interrupts(void){
    CNEN1bits.CN14IE = 1;  //sets the second bit of CNEN1, CN1IE (change notif 1 interrupt enable)
    IFS1bits.CNIF = 0; //make sure the interrupt flag is set low
    IEC1bits.CNIE = 1; //make sure all change notif inputs are on

    // enable OC3 interrupt
    IEC1bits.OC3IE = 1; 
    //Set OC interrupt flags low
    IFS1bits.OC3IF = 0;
}

void init_motor(void){

    //outputs
    pin_digitalOut(IN1); //D2-bar
    pin_write(IN1,1);

    pin_digitalOut(IN2); //D2-bar
    pin_write(IN2,0);

    pin_digitalOut(D1); //D1
    pin_write(D1,0); //no tri-stating!

    pin_digitalOut(ENA); //ENA
    pin_write(ENA,1); //Enable the system

    pin_digitalOut(&D[7]); //SLEW
    pin_write(&D[7],0); //low slew rate

    pin_digitalOut(INV); //INV
    pin_write(INV,0); //don't invert the inputs!    

    //inputs
    pin_analogIn(CURRENT_PIN); //direction sensor
    pin_analogIn(VEMF_PIN); //Vemf sensor
    pin_analogIn(FB_PIN); //0.24% of active high side current
    pin_digitalIn(REV_PIN); //tach input
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

void get_direction(void){
    uint16_t d = pin_read(CURRENT_PIN);
    if (d >= 0x8000){
        SENSED_DIRECTION = 0;
    } else {
        SENSED_DIRECTION = 1;
    }
}

void get_feedback(void){
    FB = pin_read(FB_PIN);
}

void SetMotorVelocity(uint16_t duty, uint16_t direction){
    pin_write(D2n,duty);
    pin_write(IN1,!direction);
    pin_write(IN2,direction);
}

void toggle_direction(void){
    REQUESTED_DIRECTION = !REQUESTED_DIRECTION;
}


int16_t main(void) {
    init();
    REV = 0;
    REQUESTED_DIRECTION = 0;
    SENSED_DIRECTION = 0;
    DUTY = 0;
    led_on(&led2);
    timer_setPeriod(HB_TIMER, 0.5);
    timer_start(HB_TIMER);
    printf("Good morning!\n");

    oc_pwm(&oc3,D2n,PWM_TIMER,FREQ,DUTY);
    
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        ServiceUSB();
        SetMotorVelocity(DUTY,REQUESTED_DIRECTION); 
        get_direction();
        get_feedback();
         if (timer_flag(HB_TIMER)) {
            timer_lower(HB_TIMER);
            led_toggle(&led1);
        }      
    }
}

