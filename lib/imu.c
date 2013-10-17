/*
 * =====================================================================================
 *
 *       Filename:  gyrotest.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/30/2013 11:14:02
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "common.h"
#include "imu.h"


_PIN IMU_MOSI, IMU_MISO, IMU_SCK, ACCEL_CS, GYRO_CS;

void imu_init(void) {
    pin_init(&IMU_MOSI, (unsigned int *)&PORTB, (unsigned int *)&TRISB, 
            (unsigned int *)NULL, 8, -1, 0, 8, (unsigned int *)&RPOR4);
    pin_init(&IMU_SCK, (unsigned int *)&PORTB, (unsigned int*)&TRISB,
            (unsigned int *)NULL, 9, -1, 8, 9, (unsigned int*)&RPOR4);
    pin_init(&IMU_MISO, (unsigned int *)&PORTB, (unsigned int *)&TRISB,
            (unsigned int *)NULL, 14, -1, 0, 14, (unsigned int *)&RPOR7);
    pin_init(&ACCEL_CS, (unsigned int*)&PORTB, (unsigned int *)NULL,
            (unsigned int *)NULL, 13, -1, 0, -1, (unsigned int *)NULL);
    pin_init(&GYRO_CS, (unsigned int *)&PORTB, (unsigned int *)NULL,
            (unsigned int *)NULL, 11, -1, 0, -1, (unsigned int *)NULL);

    pin_digitalOut(&ACCEL_CS);
    pin_digitalOut(&GYRO_CS);

    spi_open(&spi1, &IMU_MISO, &IMU_MOSI, &IMU_SCK, 2e6);

    accel_write(I2CADD, 0x80);        //Disable I2C
}

void gyro_write(unsigned char address, unsigned char value){
    pin_clear(&GYRO_CS);
    spi_transfer(&spi1, address&0x3F);
    spi_transfer(&spi1, value&0xFF);
    pin_set(&GYRO_CS);
}

unsigned char gyro_read(unsigned char address) {
    unsigned char result;
    pin_clear(&GYRO_CS);
    spi_transfer(&spi1, 0x80|(address&0x3F));
    result = spi_transfer(&spi1, 0x00);
    pin_set(&GYRO_CS);
    return result;
}


void accel_write(unsigned char address, unsigned char value) {
    pin_clear(&ACCEL_CS);
    spi_transfer(&spi1, 0x80|((address&0x3F)<<1));
    spi_transfer(&spi1,value&0xFF);
    pin_set(&ACCEL_CS);
}

unsigned char accel_read(unsigned char address) {
    unsigned char result;
    pin_clear(&ACCEL_CS);
    spi_transfer(&spi1, (address&0x3F)<<1);
    result = spi_transfer(&spi1, 0x00);
    pin_set(&ACCEL_CS);
    return result;
}

void accel_set_measure_mode(void) {
    accel_write(MCTL, 0x05);              //2g measurement mode
    accel_write(CTL1, 0x80); //set absolute condition, 125Hz BW, all axes enabled
}

void accel_read_xyz(int * data){
    data[0] = (int)(signed char)accel_read(XOUT8);
    data[1] = (int)(signed char)accel_read(YOUT8);
    data[2] = (int)(signed char)accel_read(ZOUT8);
}

void accel_calibrate(int * offsets) {
    int xcal, ycal, zcal, z;
    
    accel_write(XOFFL, 0);
    accel_write(XOFFH, 0);
    accel_write(YOFFL, 0);
    accel_write(YOFFH, 0);
    accel_write(ZOFFL, 0);
    accel_write(ZOFFH, 0);

    accel_read_xyz((int *)accel_xyz);
    xcal = -2 * accel_xyz[0];
    xcal = xcal < 0 ? xcal + 2048 : xcal;
    ycal = -2 * accel_xyz[1];
    ycal = ycal < 0 ? ycal + 2048 : ycal;
    zcal = -2 * (accel_xyz[2] - 64);
    zcal = zcal < 0 ? zcal + 2048 : zcal;

    accel_write(XOFFL, xcal&0x00FF);
    accel_write(XOFFH, xcal>>8);
    accel_write(YOFFL, ycal&0x00FF);
    accel_write(YOFFH, ycal>>8);
    accel_write(ZOFFL, zcal&0x00FF);
    accel_write(ZOFFH, zcal>>8);

    offsets[0] = xcal;
    offsets[1] = ycal;
    offsets[2] = zcal;
}
